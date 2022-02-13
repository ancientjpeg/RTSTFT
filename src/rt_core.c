/**
 * @file rt_core.c
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 * - General plan
 *   - user init
 *     - user creates an rt_params_t struct, passes it to be initialized
 *     - user is responsible for defining frame_size, block_size,
 * overlap_factor, pad_factor
 * - current issues
 *   - STFT aliasing
 *     - even at 1024 fft-size, severe aliasing w/ sine waves
 *       - 1024 fft gives 512 real bands
 *       - 1kHz -> 1.26 kHz (major third)
 *         - 1kHz aliased between 990 and 1033 Hz
 *         - 1.26kHz aliased between 1248 and 1290
 *   - pitch issues
 *     - even without clear aliasing, serious pitch drift sometimes
 */
#include "rtstft.h"

/*

*/

void rt_digest_frame(rt_params p, rt_chan c)
{
  rt_uint  this_frame = c->framebuf->next_write;
  rt_real *frame_ptr  = c->framebuf->frames[this_frame];
  if (p->pad_factor != 0) {
    rt_uint i;
    for (i = 0; i < p->pad_offset; i++) {
      frame_ptr[i]                   = 0.;
      frame_ptr[p->fft_size - 1 - i] = 0.;
    }
    for (i = 0; i < p->fft_size; i++) {
      frame_ptr[i] = 0.;
    }
  }
  rt_fifo_dequeue_staggered(c->in, frame_ptr + p->pad_offset, p->frame_size,
                            p->hop_a);
  c->framebuf->frame_data[this_frame] |= RT_FRAME_IS_FILLED;
  c->framebuf->next_write =
      rt_framebuf_relative_frame(c->framebuf, this_frame, 1);
  rt_framebuf_convert_frame(p, c, this_frame);
}

void rt_process_frame(rt_params p, rt_chan c)
{
  rt_uint this_frame = c->framebuf->next_unprocessed;
  rt_framebuf_process_frame(p, c, this_frame);
}

void rt_assemble_frame(rt_params p, rt_chan c)
{
  rt_uint this_frame = c->framebuf->next_unread;
  rt_framebuf_revert_frame(p, c, this_frame);
  if (c->framebuf->frame_data[this_frame] & RT_FRAME_IS_INVERTED) {
    rt_fifo_enqueue_staggered(c->pre_lerp,
                              c->framebuf->frames[this_frame] + p->pad_offset,
                              p->frame_size, p->hop_s);
    c->framebuf->next_unread =
        rt_framebuf_relative_frame(c->framebuf, this_frame, 1);
  }
  else {
    fprintf(stderr, "Frame error in assemble.\n");
  }
}

void rt_lerp_read_out(rt_params p, rt_chan c, rt_uint num_hops)
{
  rt_uint input_size  = p->hop_s * num_hops;
  rt_uint output_size = p->hop_a * num_hops;
  rt_real local_incr  = (rt_real)(input_size - 1) / (output_size - 1);
  rt_real pos         = 0.;
  rt_uint i;
  for (i = 0; i < output_size; i++) {
    rt_uint x0       = (rt_uint)(i == output_size - 1 ? pos : round(pos));
    rt_real mod      = pos - x0;
    rt_uint x0_index = rt_fifo_new_pos(c->pre_lerp, c->pre_lerp->head, x0);
    rt_uint x1_index = rt_fifo_new_pos(c->pre_lerp, c->pre_lerp->head, x0 + 1);
    rt_real y0       = c->pre_lerp->queue[x0_index];
    rt_real y1       = c->pre_lerp->queue[x1_index];
    rt_real result   = (y1 - y0) * mod + y0;

    rt_fifo_enqueue_one(c->out, &result);
    pos += local_incr;
  }

  rt_fifo_dequeue(c->pre_lerp, input_size);
}

void rt_cycle_chan(rt_params p, rt_uint channel_index, rt_real *buffer,
                   rt_uint buffer_len)
{
  rt_chan  c               = p->chans[channel_index];
  rt_real *buffer_orig     = buffer;
  rt_uint  buffer_len_save = buffer_len;
  while (buffer_len > 0) {
    rt_fifo_enqueue_one(c->in, buffer);
    char check = 0;

    while (rt_fifo_payload(c->in) >= p->frame_size) {
      rt_digest_frame(p, c);
      rt_process_frame(p, c);
      if (!c->first_frame) {
        rt_assemble_frame(p, c);
      }
      else {
        c->first_frame = 0;
      }
      if (rt_fifo_readable(c->pre_lerp) >= p->hop_s * p->overlap_factor) {
        rt_lerp_read_out(p, c, p->overlap_factor);
      }
      if (++check > 1) {
        fprintf(stderr, "Extracted more than one frame.\n");
        exit(1);
      }
    }
    if (rt_fifo_readable(c->out)) {
      rt_fifo_dequeue_one(c->out, buffer);
    }
    else {
      *buffer = 0.;
    }
    ++buffer;
    --buffer_len;
  }
}
