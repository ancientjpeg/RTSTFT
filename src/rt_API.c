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

rt_params rt_init(rt_uint num_channels, rt_uint frame_size, rt_uint buffer_size,
                  rt_uint overlap_factor, rt_uint pad_factor, float sample_rate)
{
  rt_params p           = malloc(sizeof(rt_params_t));
  p->initialized        = 0;
  p->fft_min            = 1UL << RT_FFT_MIN_POW; /** 2 * SIMD_SZ ^ 2 */
  p->fft_max            = 1UL << RT_FFT_MAX_POW;
  p->scale_factor_max   = 2.0;
  p->scale_factor_min   = 1. / p->scale_factor_max;
  p->manip_multichannel = 0; /* implement multichannel manip later plz */
  p->num_chans          = num_channels;
  rt_holder_init(p, num_channels, frame_size, buffer_size, overlap_factor,
                 pad_factor, sample_rate);
  rt_update_params(p);
  p->phase_mod = 1.0;
  rt_parser_clear_buffer(&(p->parser));
  p->chans = malloc(p->num_chans * sizeof(rt_chan));
  rt_uint i;
  for (i = 0; i < p->num_chans; i++) {
    p->chans[i] = rt_chan_init(p);
  }
  p->initialized = 1;
  p->listener    = (rt_listener_t){};
  return p;
}

void rt_update_params(rt_params p)
{
  const rt_holder h = p->hold;

  p->frame_size     = h->frame_size;
  p->fft_size       = h->fft_size;
  p->overlap_factor = h->overlap_factor;
  p->pad_factor     = h->pad_factor;
  p->pad_offset     = (p->fft_size - p->frame_size) / 2;
  p->setup          = h->setup;
  p->sample_rate    = h->sample_rate;

  p->scale_factor   = h->scale_factor;
  p->retention_mod  = h->retention_mod;
  p->phase_mod      = h->phase_mod;
  p->phase_chaos    = h->phase_chaos;

  p->buffer_size    = h->buffer_size;
  p->hop_a          = p->frame_size / p->overlap_factor;
  p->hop_s          = lround(p->hop_a * p->scale_factor);
  rt_uint i;
  for (i = 1; i < RT_MANIP_FLAVOR_COUNT; i++) {
    p->enabled_manips
        |= 1 << i; /**< sets all manipulation ON, except multichannel */
  }
  if ((p->hold->tracker & RT_MANIPS_CHANGED) && p->initialized) {
    rt_update_manips(p);
  }
  p->hold->tracker = 0;
}

void rt_flush(rt_params p)
{
  rt_uint i;
  for (i = 0; i < p->num_chans; i++) {
    rt_fifo_flush(p->chans[i]->in);
    rt_fifo_flush(p->chans[i]->out);
    rt_framebuf_flush(p, p->chans[i]->framebuf);
  }
}
rt_params rt_clean(rt_params p)
{
  rt_uint i;
  for (i = 0; i < p->num_chans; i++) {
    rt_chan_clean(p, p->chans[i]);
  }
  free(p->chans);
  rt_holder_clean(p->hold);
  free(p);
  return (rt_params)NULL;
}

void rt_start_cycle(rt_params p)
{
  p->cycle_info |= RT_IN_CYCLE;
  p->cycle_info |= RT_AT_CYCLE_START;
  rt_update_params(p);
}
void rt_end_cycle(rt_params p) { p->cycle_info = 0; }

void rt_cycle_single(rt_params p, rt_real *buffer, rt_uint buffer_len)
{
  rt_cycle_offset(p, &buffer, 1, buffer_len, 0);
}

/**
 * @brief The simplest interface for cycling RTSTFT over a set of input buffers.
 *
 * @param p rt_params object representing the active RTSTFT instance.
 * @param buffers Array of rt_real buffers--should have as many buffers as
 * RTSTFT has channels.
 * @param buffer_len The length of the supplied buffers.
 *
 * Do note: rt_cycle is the only API cycling function that will call
 * rt_start_cycle() and rt_end_cycle() for you. When using all other cycling
 * functions, these must be called manually.
 */
void rt_cycle(rt_params p, rt_real **buffers, rt_uint buffer_len)
{
  rt_start_cycle(p);
  rt_cycle_offset(p, buffers, p->num_chans, buffer_len, 0);
  rt_end_cycle(p);
}

void rt_cycle_offset(rt_params p, rt_real **buffers, rt_uint num_buffers,
                     rt_uint buffer_len, rt_uint sample_offset)
{
  rt_uint i;
  for (i = 0; i < num_buffers; i++) {
    rt_cycle_chan(p, i, buffers[i] + sample_offset, buffer_len);
  }
}

rt_uint rt_check_pow_2(rt_uint num)
{
  rt_uint i = 0, check;
  do {
    check = 1UL << i;
    if (check & num) {
      if (~check & num) {
        fprintf(stderr,
                "FFT size must be a power of two. Supplied value: %lu\n", num);
        return RT_UINT_FALSE;
      }
      return i;
    }
  } while (i++ < RT_FFT_MAX_POW);

  fprintf(stderr, "%lu is an invalid frame size. Must be greater than %lu.\n",
          num, 1UL << RT_FFT_MIN_POW);
  return RT_UINT_FALSE;
}
