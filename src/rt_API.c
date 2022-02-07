/**
 * @file rt_API.c
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "rtstft.h"
#define rt_max(a, b) ((a) > (b) ? (a) : (b))
#define rt_min(a, b) ((a) < (b) ? (a) : (b))

rt_chan   rt_chan_init(rt_params p);
rt_chan   rt_chan_clean(rt_params p, rt_chan chan);

rt_params rt_init(rt_uint num_channels, rt_uint frame_size_pow,
                  rt_uint buffer_size_pow, rt_uint overlap_factor,
                  rt_uint pad_factor, float sample_rate)
{
  rt_params p     = malloc(sizeof(rt_params_t));
  p->fft_min_pow  = 5; /** 2 * SIMD_SZ ^ 2 */
  p->fft_max_pow  = 16;
  p->pad_factor   = pad_factor;
  p->fft_curr_pow = frame_size_pow + pad_factor;
  if (p->fft_curr_pow > p->fft_max_pow) {
    fprintf(stderr, "Exceeded maximum frame size.\n");
    exit(1);
  }
  else if (p->fft_curr_pow < p->fft_min_pow) {
    fprintf(stderr, "Below minimum frame size.\n");
    exit(1);
  }

  p->scale_factor   = 1.;
  p->frame_size     = 1 << frame_size_pow;
  p->fft_size       = 1 << p->fft_curr_pow;
  p->pad_offset     = (p->fft_size - p->frame_size) / 2;
  p->frame_max      = 1 << 15;

  p->overlap_factor = overlap_factor;
  p->buffer_size    = 1U << buffer_size_pow;
  p->sample_rate    = sample_rate;
  p->hop_a          = p->frame_size / p->overlap_factor;
  p->hop_s          = lround(p->hop_a * p->scale_factor);
  p->num_chans      = num_channels;
  p->chans          = malloc(p->num_chans * sizeof(rt_chan));
  rt_uint i;
  for (i = 1; i < RT_MANIP_TYPE_COUNT; i++) {
    p->manip_settings |=
        1 << i; /**< sets all manipulation ON, except multichannel */
  }
  /*   p->manip_settings = 0; // debug kill switch */
  for (i = 0; i < p->num_chans; i++) {
    p->chans[i] = rt_chan_init(p);
  }
  return p;
}

rt_params rt_clean(rt_params p)
{
  rt_uint i;
  for (i = 0; i < p->num_chans; i++) {
    rt_chan_clean(p, p->chans[i]);
  }
  free(p->chans);
  free(p);
  return (rt_params)NULL;
}

void rt_cycle_single(rt_params p, rt_real *buffer, rt_uint buffer_len)
{
  rt_cycle_chan(p, 0, buffer, buffer_len);
}

void rt_cycle(rt_params p, rt_real **buffers, rt_uint num_buffers,
              rt_uint buffer_len)
{
  rt_cycle_offset(p, buffers, num_buffers, buffer_len, 0);
}

void rt_cycle_offset(rt_params p, rt_real **buffers, rt_uint num_buffers,
                     rt_uint buffer_len, rt_uint sample_offset)
{
  rt_uint i;
  for (i = 0; i < num_buffers; i++) {
    rt_cycle_chan(p, i, buffers[i] + sample_offset, buffer_len);
  }
}

rt_chan rt_chan_init(rt_params p)
{
  rt_chan chan       = (rt_chan)malloc(sizeof(rt_chan_t));
  chan->first_frame  = 1;
  chan->in           = rt_fifo_init(rt_max(p->buffer_size, p->frame_size * 2));
  rt_uint lerp_frame = p->overlap_factor * p->hop_s + p->frame_size;
  chan->pre_lerp     = rt_fifo_init(
          rt_max((rt_uint)ceil((rt_real)p->buffer_size * 2 * p->scale_factor),
                 lerp_frame * 2));
  chan->out      = rt_fifo_init(rt_max(p->buffer_size, p->frame_size * 2));
  chan->manips   = rt_manip_init(p, chan);
  chan->framebuf = rt_framebuf_init(p, 2);
  return chan;
}

rt_chan rt_chan_clean(rt_params p, rt_chan chan)
{
  chan->framebuf = rt_framebuf_destroy(p, chan->framebuf);
  chan->in       = rt_fifo_destroy(chan->in);
  chan->pre_lerp = rt_fifo_destroy(chan->pre_lerp);
  chan->out      = rt_fifo_destroy(chan->out);
  free(chan);
  return (rt_chan)NULL;
}

rt_uint rt_real_size() { return sizeof(rt_real); }