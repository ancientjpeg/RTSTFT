#include "rtstft.h"

/*
  - General plan
    - user init
      - user creates an rt_params_t struct, passes it to be initialized
      - user is responsible for defining frame_size, block_size, overlap_factor
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
  rt_params p        = malloc(sizeof(rt_params_t));
  p->scale_factor    = scale_factor;
  p->frame_size      = frame_size;
  p->overlap_factor  = overlap_factor;
  p->buffer_size     = buffer_size;
  p->sample_rate     = sample_rate;
  p->latency_block   = p->frame_size * p->overlap_factor;
  p->hop_a           = p->frame_size / p->overlap_factor;
  p->hop_s           = lround(p->hop_a * p->scale_factor);
  rt_uint num_frames = 2; /* THIS COULD BE WRONG */
  p->framebuf        = rt_framebuf_init(p, num_frames);
  p->plan            = fftw_plan_r2r_1d(p->frame_size, p->framebuf->frames[0],
                                        p->framebuf->frames[0], FFTW_R2HC, FFTW_ESTIMATE);
  p->plan_inv =
      fftw_plan_r2r_1d(p->frame_size, p->framebuf->frames[0],
                       p->framebuf->frames[0], FFTW_HC2R, FFTW_ESTIMATE);
  rt_uint lerp_frame = p->overlap_factor * p->hop_s * 2;
  p->in              = rt_fifo_init(rt_max(p->buffer_size, p->frame_size * 2));
  p->pre_lerp    = rt_fifo_init(rt_max(ceil(p->buffer_size * p->scale_factor) +
                                           lerp_frame * p->overlap_factor,
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
  rt_fifo_dequeue_staggered(p->in, p->framebuf->frames[this_frame],
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
  rt_uint last_frame = rt_framebuf_relative_frame(p->framebuf, this_frame, -1);
  rt_framebuf_revert_frame(p, this_frame);
  if (p->framebuf->frame_data[this_frame] & RT_FRAME_IS_INVERTED) {
    rt_fifo_enqueue_staggered(p->pre_lerp, p->framebuf->frames[this_frame],
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
  /*  p->hop_pos % hop_s will ALWAYS equal
      floor(hop_s / hop_a) * (lifetime buffer sample count)) % hop_s
      with the help of p->mod_track   */
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
    rt_fifo_enqueue(p->out, &result, 1);
  }
  rt_fifo_enqueue(p->out,
                  p->pre_lerp->queue +
                      rt_fifo_new_pos(p->out, p->out->head, input_size - 1),
                  1);

  rt_fifo_dequeue(p->pre_lerp, input_size);
}

void rt_cycle(rt_params p, rt_real *buffer, rt_uint buffer_len)
{
  rt_uint buffer_len_save = buffer_len;
  while (buffer_len > 0) {
    rt_fifo_enqueue(p->in, buffer, 1);
    char check = 0;
    while (rt_fifo_payload(p->in) >= p->frame_size) {
      rt_digest_frame(p);
      char was_first = p->first_frame;
      rt_process_frame(p);
      if (!was_first) {
        rt_assemble_frame(p);
      }
      if (rt_fifo_readable(p->pre_lerp) >= p->overlap_factor * p->hop_s) {
        rt_lerp_read_out(p, p->overlap_factor);
      }
      if (++check > 1) {
        fprintf(stderr, "Extracted more than one frame.\n");
        exit(1);
      }
    }
    rt_uint i = buffer_len_save - buffer_len;
    if (rt_fifo_readable(p->out)) {
      rt_uint temp = rt_fifo_readable(p->out);
      rt_fifo_dequeue_staggered(p->out, buffer, 1, 1);
    }
    else {
      *buffer = 0.;
    }
    ++buffer;
    --buffer_len;
  }
}
