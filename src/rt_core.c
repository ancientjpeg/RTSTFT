/**
 * @file rt_core.c
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
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

/**
 * @brief Digest (i.e. transform, process, invert) a frame from p->in, then lerp
 * and overlap-add it to p->out.
 *
 * @param p An rt_params signifying the active instance of RTSTFT.
 * @param c An rt_chan signifying the currently active channel.
 */
void rt_digest_frame(rt_params p, rt_chan c)
{
  rt_real *frame_ptr = c->framebuf->frame;
  if (p->pad_factor != 0) {
    rt_uint i;
    for (i = 0; i < p->pad_offset; i++) {
      frame_ptr[i]                   = 0.;
      frame_ptr[p->fft_size - 1 - i] = 0.;
    }
  }
  rt_fifo_dequeue_staggered(c->in, frame_ptr + p->pad_offset, p->frame_size,
                            p->hop_a);
  rt_framebuf_digest_frame(p, c);

  /** to lerp directly from framebuf to c->out, we do a slightly backwards
   * lerp, i.e. we write to the next hop_a position but write N /
   * scale_factor samples.
   */
  rt_uint input_size    = p->frame_size;
  rt_uint hop_s_inverse = round(p->hop_a / p->scale_factor);
  rt_uint output_size   = hop_s_inverse * p->overlap_factor;
  rt_real pos = 0., local_incr = (rt_real)(input_size - 1) / (output_size - 1);
  rt_uint out_pos_init = c->out->write_pos;
  rt_uint i, x0, x1;
  rt_real mod, y0, y1, result;
  rt_fifo_enqueue_one(c->out, c->framebuf->frame[0]);
  for (i = 0; i < output_size - 1; i++) {
    x0     = floor(pos);
    x1     = x0 + 1;
    mod    = pos - x0;
    y0     = c->framebuf->frame[x0];
    y1     = c->framebuf->frame[x1];
    result = (y1 - y0) * mod + y0;
    rt_fifo_enqueue_one(c->out, result);
    pos += local_incr;
  }
  rt_fifo_enqueue_one(c->out, c->framebuf->frame[input_size - 1]);
  c->out->write_pos = rt_fifo_new_pos(c->out, out_pos_init, p->hop_a);
}

void rt_cycle_chan(rt_params p, rt_uint channel_index, rt_real *buffer,
                   rt_uint buffer_len)
{
  rt_chan c = p->chans[channel_index];
  while (buffer_len > 0) {
    if (!p->initialized) {
      *buffer = 0.f;
    }
    else {
      rt_fifo_enqueue_one(c->in, *buffer);
      if (rt_fifo_payload(c->in) >= p->frame_size) {
        rt_params_check_mod(p);
        rt_digest_frame(p, c);
      }

      if (rt_fifo_readable(c->out)) {
        rt_fifo_dequeue_one(c->out, buffer);
      }
      else {
        *buffer = 0.;
      }
    }
    ++buffer;
    --buffer_len;
  }
}

void rt_params_check_mod(rt_params p)
{
  if (p->hold->tracker) {
    rt_update_params(p);
  }
}
