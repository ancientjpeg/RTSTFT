#include "rtstft.h"

/*
  - General plan
    - user init
      - user creates an rt_params_t struct, passes it to be initialized
      - user is responsible for defining fft_size, block_size, overlap_factor
*/

#define rt_max(a, b) ((a) > (b) ? (a) : (b))
#define rt_min(a, b) ((a) < (b) ? (a) : (b))

rt_params rt_init(rt_uint frame_size, rt_uint overlap_factor,
                  rt_uint buffer_size, float sample_rate, float scale_factor)
{
  if (frame_size * sizeof(float) % 16 != 0) {
    fprintf(stderr, "Frames  must be able to by byte-aligned to 16 bytes.");
    exit(1);
  }
  else if (!frame_size || overlap_factor <= 0.f) {
    fprintf(stderr, "Cannot have frame size or overlap factor be <= zero.");
    exit(1);
  }
  rt_params p     = malloc(sizeof(rt_params_t));
  p->scale_factor = scale_factor;
  p->pad_factor   = 0;
  p->frame_size   = frame_size;
  p->fft_size     = frame_size * (1 << p->pad_factor);
  p->pad_offset   = (p->fft_size - p->frame_size) / 2;
  p->frame_max    = 1 << 15;
  if (p->fft_size > p->frame_max) {
    fprintf(stderr, "Exceeded maximum frame size.\n");
    exit(1);
  }
  else if (p->fft_size < 32) {
    fprintf(stderr, "Below minimum frame size.\n");
    exit(1);
  }
  p->overlap_factor = overlap_factor;
  p->buffer_size    = buffer_size;
  p->sample_rate    = sample_rate;
  p->hop_a          = p->frame_size / p->overlap_factor;
  p->hop_s          = lround(p->hop_a * p->scale_factor);
  p->framebuf       = rt_framebuf_init(p, 2);
  p->plan           = fftw_plan_r2r_1d(p->fft_size, p->framebuf->frames[0],
                                       p->framebuf->frames[0], FFTW_R2HC, FFTW_ESTIMATE);
  p->plan_inv =
      fftw_plan_r2r_1d(p->fft_size, p->framebuf->frames[0],
                       p->framebuf->frames[0], FFTW_HC2R, FFTW_ESTIMATE);
  p->in              = rt_fifo_init(rt_max(p->buffer_size, p->frame_size * 2));
  rt_uint lerp_frame = p->overlap_factor * p->hop_s + p->frame_size;
  p->pre_lerp        = rt_fifo_init(
             rt_max((rt_uint)ceil((rt_real)p->buffer_size * 2 * scale_factor),
                    lerp_frame * 2));
  p->out         = rt_fifo_init(rt_max(p->buffer_size, p->frame_size * 2));
  p->first_frame = 1;
  return p;
}

rt_params rt_clean(rt_params p)
{
  p->framebuf = rt_framebuf_destroy(p->framebuf);
  p->in       = rt_fifo_destroy(p->in);
  p->pre_lerp = rt_fifo_destroy(p->pre_lerp);
  fftw_destroy_plan(p->plan);
  fftw_destroy_plan(p->plan_inv);
  free(p);
  return (rt_params)NULL;
}

void rt_digest_frame(rt_params p)
{
  rt_uint this_frame = p->framebuf->next_write;
  if (p->pad_factor) {
    rt_uint i;
    for (i = 0; i < p->pad_offset; i++) {
      p->framebuf->frames[this_frame][i]                   = 0.;
      p->framebuf->frames[this_frame][p->fft_size - 1 - i] = 0.;
    }
  }
  rt_fifo_dequeue_staggered(p->in,
                            p->framebuf->frames[this_frame] + p->pad_offset,
                            p->frame_size, p->hop_a);
  p->framebuf->frame_data[this_frame] |= RT_FRAME_IS_FILLED;
  p->framebuf->next_write =
      rt_framebuf_relative_frame(p->framebuf, this_frame, 1);
  rt_framebuf_convert_frame(p, this_frame);
}
void rt_process_frame(rt_params p)
{
  rt_uint this_frame = p->framebuf->next_unprocessed;
  rt_framebuf_process_frame(p, this_frame);
}

void rt_assemble_frame(rt_params p)
{
  rt_uint this_frame = p->framebuf->next_unread;
  rt_framebuf_revert_frame(p, this_frame);
  if (p->framebuf->frame_data[this_frame] & RT_FRAME_IS_INVERTED) {
    rt_fifo_enqueue_staggered(p->pre_lerp,
                              p->framebuf->frames[this_frame] + p->pad_offset,
                              p->frame_size, p->hop_s);
    p->framebuf->next_unread =
        rt_framebuf_relative_frame(p->framebuf, this_frame, 1);
  }
  else {
    fprintf(stderr, "Frame error in assemble.\n");
  }
}

void rt_lerp_read_out(rt_params p, rt_uint num_hops)
{
  rt_uint          input_size  = p->hop_s * num_hops;
  rt_uint          output_size = p->hop_a * num_hops;
  register rt_real local_incr  = (rt_real)(input_size - 1) / (output_size - 1);
  register rt_uint i;
  for (i = 0; i < output_size - 1; i++) {
    rt_real pos       = i * local_incr;
    rt_uint pos_floor = (rt_uint)pos;
    rt_real mod       = pos - pos_floor;
    rt_uint i_input =
        rt_fifo_new_pos(p->pre_lerp, p->pre_lerp->head, pos_floor);
    rt_uint i_input_1 =
        rt_fifo_new_pos(p->pre_lerp, p->pre_lerp->head, pos_floor + 1);
    rt_real result =
        (p->pre_lerp->queue[i_input_1] - p->pre_lerp->queue[i_input]) * mod +
        p->pre_lerp->queue[i_input];

    rt_fifo_enqueue_one(p->out, &result);
  }
  rt_fifo_enqueue_one(p->out,
                      p->pre_lerp->queue + rt_fifo_new_pos(p->pre_lerp,
                                                           p->pre_lerp->head,
                                                           input_size - 1));

  rt_fifo_dequeue(p->pre_lerp, input_size);
}

void rt_cycle(rt_params p, rt_real *buffer, rt_uint buffer_len)
{
  rt_real *buffer_orig     = buffer;
  rt_uint  buffer_len_save = buffer_len;
  while (buffer_len > 0) {
    rt_fifo_enqueue_one(p->in, buffer);
    char check = 0;

    while (rt_fifo_payload(p->in) >= p->frame_size) {
      rt_digest_frame(p);
      char was_first = p->first_frame;
      rt_process_frame(p);
      if (!was_first) {
        rt_assemble_frame(p);
      }
      if (rt_fifo_readable(p->pre_lerp) >= p->hop_s * p->overlap_factor) {
        rt_lerp_read_out(p, p->overlap_factor);
      }
      if (++check > 1) {
        fprintf(stderr, "Extracted more than one frame.\n");
        exit(1);
      }
    }
    if (rt_fifo_readable(p->out)) {
      rt_fifo_dequeue_one(p->out, buffer);
    }
    else {
      *buffer = 0.;
    }
    ++buffer;
    --buffer_len;
  }
}
