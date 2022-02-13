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

void rt_set_params(rt_params p, rt_uint frame_size_pow, rt_uint buffer_size_pow,
                   rt_uint overlap_factor, rt_uint pad_factor,
                   rt_real scale_factor, char init)
{
  p->pad_factor   = pad_factor;
  p->fft_size_pow = frame_size_pow + pad_factor;
  if (p->fft_size_pow > p->fft_max_pow) {
    fprintf(stderr, "Exceeded maximum frame size.\n");
    exit(1);
  }
  else if (p->fft_size_pow < p->fft_min_pow) {
    fprintf(stderr, "Below minimum frame size.\n");
    exit(1);
  }
  p->frame_size   = 1 << frame_size_pow;
  p->fft_size     = 1 << p->fft_size_pow;
  p->pad_offset   = (p->fft_size - p->frame_size) / 2;

  p->phase_modif  = 1.0;
  p->scale_factor = scale_factor;
  if (p->scale_factor > p->scale_factor_max ||
      p->scale_factor < p->scale_factor_min) {
    fprintf(stderr,
            "Scale factor must be in range %.1f-%.1f. Found value %.4f\n",
            p->scale_factor_max, p->scale_factor_min, p->scale_factor);
    exit(1);
  }

  p->overlap_factor = overlap_factor;
  p->buffer_size    = 1U << buffer_size_pow;
  p->hop_a          = p->frame_size / p->overlap_factor;
  p->hop_s          = lround(p->hop_a * p->scale_factor);
  rt_uint i;
  for (i = 1; i < RT_MANIP_TYPE_COUNT; i++) {
    /* do not forget to set appropriate manips ! */
  }
}

rt_params rt_init(rt_uint num_channels, rt_uint frame_size_pow,
                  rt_uint buffer_size_pow, rt_uint overlap_factor,
                  rt_uint pad_factor, float sample_rate)
{
  rt_params p           = malloc(sizeof(rt_params_t));
  p->fft_min_pow        = 5; /** 2 * SIMD_SZ ^ 2 */
  p->fft_max_pow        = 16;
  p->fft_max_size       = 1 << p->fft_max_pow;
  p->scale_factor_max   = 2.0;
  p->scale_factor_min   = 1. / p->scale_factor_max;
  p->sample_rate        = sample_rate;
  p->num_chans          = num_channels;
  p->manip_multichannel = 0; /* implement multichannel manip later plz */

  rt_set_params(p, frame_size_pow, buffer_size_pow, overlap_factor, pad_factor,
                0.7, 1);

  p->chans = malloc(p->num_chans * sizeof(rt_chan));
  rt_uint i;
  for (i = 1; i < RT_MANIP_TYPE_COUNT; i++) {
    p->manip_settings |=
        1 << i; /**< sets all manipulation ON, except multichannel */
  }
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
  rt_cycle_offset(p, &buffer, 1, buffer_len, 0);
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
  rt_chan chan = (rt_chan)malloc(sizeof(rt_chan_t));
  chan->in     = rt_fifo_init(rt_max(p->buffer_size, p->fft_max_size * 2));
  rt_uint lerp_frame = p->overlap_factor * p->hop_s + p->fft_max_size;
  chan->pre_lerp     = rt_fifo_init(
          rt_max((rt_uint)ceil((rt_real)p->buffer_size * 2 * p->scale_factor_max),
                 lerp_frame * 2));
  chan->out      = rt_fifo_init(rt_max(p->buffer_size, p->fft_max_size * 2));
  chan->manips   = rt_manip_init(p, chan);
  chan->framebuf = rt_framebuf_init(p);
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