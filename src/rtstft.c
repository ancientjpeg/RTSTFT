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
                  rt_uint buffer_size, float sample_rate)
{
  if (frame_size * sizeof(float) % 16 != 0) {
    fprintf(stderr, "Frames  must be able to by byte-aligned to 16 bytes.");
    exit(1);
  }
  else if (!frame_size || overlap_factor <= 0.f) {
    fprintf(stderr, "Cannot have frame size or overlap factor be <= zero.");
    exit(1);
  }
  rt_params p            = malloc(sizeof(rt_params_t));
  p->scale_factor        = 1.0;
  p->frame_size          = frame_size;
  p->overlap_factor      = overlap_factor;
  p->buffer_size         = buffer_size;
  p->sample_rate         = sample_rate;
  rt_uint max_framesize  = 1 << 13;
  p->latency_block       = rt_max(2 * buffer_size, max_framesize);
  p->hop_a               = p->frame_size / p->overlap_factor;
  p->hop_s               = lround(p->hop_a * p->scale_factor);
  p->scale_factor_actual = (rt_real)p->hop_s / p->hop_a;
  rt_uint num_frames     = p->overlap_factor +
                       ceil((float)(rt_max(p->buffer_size, p->latency_block) /
                                    p->hop_a)); /* THIS COULD BE WRONG */
  p->block       = rt_block_init(p, num_frames);
  p->plan        = fftw_plan_r2r_1d(p->frame_size, p->block->frames[0],
                                    p->block->frames[0], FFTW_R2HC, FFTW_ESTIMATE);
  p->plan_inv    = fftw_plan_r2r_1d(p->frame_size, p->block->frames[0],
                                    p->block->frames[0], FFTW_HC2R, FFTW_ESTIMATE);
  p->in          = rt_fifo_init(2 * (num_frames + 1) * p->hop_a);
  p->out         = rt_fifo_init(2 * (num_frames + 3) * p->hop_s);
  p->first_frame = 1;
  return p;
}

rt_params rt_clean(rt_params p)
{
  p->block = rt_block_destroy(p->block);
  p->in    = rt_fifo_destroy(p->in);
  p->out   = rt_fifo_destroy(p->out);
  fftw_destroy_plan(p->plan);
  fftw_destroy_plan(p->plan_inv);
  free(p);
  return (rt_params)NULL;
}

void rt_digest_frame(rt_params p)
{
  int this_frame = p->block->next_write;
  int last_frame =
      rt_block_relative_frame(p->block->num_frames, this_frame, -1);
  if (p->block->frame_data[this_frame] & RT_FRAME_IS_FILLED) {
    fprintf(stderr, "tried to write an occupied frame.\n");
    exit(1);
  }
  rt_fifo_dequeue_staggered(p->in, p->block->frames[this_frame], p->frame_size,
                            p->hop_a);
  p->block->frame_data[this_frame] |= RT_FRAME_IS_FILLED;
  rt_hanning(p->block->frames[this_frame], p->frame_size);
  fftw_execute_r2r(p->plan, p->block->frames[this_frame],
                   p->block->frames[this_frame]);
  p->block->frame_data[this_frame] |= RT_FRAME_IS_TRANSFORMED;
  rt_block_convert_frame(p, this_frame);
  p->block->frame_data[this_frame] |= RT_FRAME_IS_CONVERTED;
  p->block->next_write =
      rt_block_relative_frame(p->block->num_frames, this_frame, 1);
}
void rt_process_frames(rt_params p)
{
  int this_frame = p->block->next_unprocessed;
  if (p->first_frame &&
      p->block->frame_data[this_frame] & RT_FRAME_IS_CONVERTED) {
    p->block->frame_data[this_frame] |= RT_FRAME_IS_PROCESSED;
    p->first_frame = 0;
    this_frame = rt_block_relative_frame(p->block->num_frames, this_frame, 1);
    p->block->next_unprocessed = this_frame;
  }
  int last_frame =
      rt_block_relative_frame(p->block->num_frames, this_frame, -1);
  while (p->block->frame_data[last_frame] & RT_FRAME_IS_PROCESSED &&
         p->block->frame_data[this_frame] & RT_FRAME_IS_TRANSFORMED) {
    rt_block_process_frame(p, this_frame);
    p->block->frame_data[this_frame] |= RT_FRAME_IS_PROCESSED;

    rt_block_revert_frame(p, last_frame);
    fftw_execute_r2r(p->plan_inv, p->block->frames[last_frame],
                     p->block->frames[last_frame]);
    rt_hanning(p->block->frames[last_frame], p->frame_size);
    rt_uint i;
    for (i = 0; i < p->frame_size; i++) {
      p->block->frames[last_frame][i] /= p->frame_size; /* normalization */
    }
    p->block->frame_data[last_frame] |= RT_FRAME_IS_INVERTED;

    last_frame = this_frame;
    this_frame = rt_block_relative_frame(p->block->num_frames, this_frame, 1);
  }
  p->block->next_unprocessed = this_frame;
}

void rt_assemble_frame(rt_params p)
{
  int this_frame = p->block->next_unread;
  while (p->block->frame_data[this_frame] & RT_FRAME_IS_INVERTED) {
    rt_fifo_enqueue_staggered(p->out, p->block->frames[this_frame],
                              p->frame_size, p->hop_s);
    p->block->frame_data[this_frame] = 0;
    this_frame = rt_block_relative_frame(p->block->num_frames, this_frame, 1);
  }
  p->block->next_unread = this_frame;
}

void rt_lerp_read_out(rt_params p, rt_real *buffer, rt_uint buffer_len,
                      rt_uint ret_size_int)
{
  /*  p->hop_pos % hop_s will ALWAYS equal
      floor(hop_s / hop_a) * (lifetime buffer sample count)) % hop_s
      with the help of p->mod_track   */
  rt_real local_incr = (rt_real)(ret_size_int - 1) / (buffer_len - 1);
  rt_uint i, index, pos;
  for (i = 0; i < buffer_len - 1; i++) {
    pos       = (rt_uint)i * local_incr;
    index     = rt_fifo_new_pos(p->out, p->out->head, pos);
    buffer[i] = p->out->queue[index];
  }
  buffer[buffer_len - 1] = p->out->queue[ret_size_int - 1];
  rt_fifo_dequeue(p->out, ret_size_int);
  p->hop_pos = (p->hop_pos + ret_size_int) % p->hop_s;
}

void rt_cycle(rt_params p, rt_real *buffer, rt_uint buffer_len)
{
  rt_fifo_enqueue(p->in, buffer, buffer_len);
  while (rt_fifo_readable_payload(p->in) >= p->frame_size) {
    rt_digest_frame(p);
    if (p->first_frame && rt_fifo_readable_payload(p->in) >= p->frame_size) {
      rt_digest_frame(p);
    }
    rt_process_frames(p);
    rt_assemble_frame(p);
  }
  while (p->latency_block > 0 && buffer_len > 0) {
    if (p->latency_block > buffer_len) {
      memset(buffer, 0, buffer_len * sizeof(rt_real));
      buffer_len = 0;
    }
    else {
      memset(buffer, 0, p->latency_block * sizeof(rt_real));
      buffer += p->latency_block;
      buffer_len -= p->latency_block;
    }
    p->latency_block = 0;
  }
  rt_uint current_payload;
  while ((current_payload = rt_fifo_readable_payload(p->out)) > p->hop_s &&
         buffer_len > 0) {
    rt_real retrieval_size = p->scale_factor_actual * buffer_len;
    rt_uint temp           = buffer_len;
    if (retrieval_size >= current_payload) {
      retrieval_size = current_payload - 1;
      temp           = floor(retrieval_size / p->scale_factor_actual);
    }
    rt_uint ret_size_int = (rt_uint)retrieval_size;
    p->mod_track += retrieval_size - ret_size_int;
    if (p->mod_track >= 1.) {
      ++ret_size_int;
      p->mod_track -= 1.;
    }
    rt_lerp_read_out(p, buffer, temp, ret_size_int);
    buffer += temp;
    buffer_len -= temp;
  }
  if (buffer_len > 0) {
    fprintf(stderr, "Critical error: buffer was not filled after cycle.\n");
    exit(1);
  }
  /*  out FIFO's readable payload must be at least 1 greater than buffer_len,
      in case mod_track increments during the read. */
}
