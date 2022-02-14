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
  rt_params p         = malloc(sizeof(rt_params_t));
  p->initialized      = 0;
  p->fft_min          = 1UL << RT_FFT_MIN_POW; /** 2 * SIMD_SZ ^ 2 */
  p->fft_max          = 1UL << RT_FFT_MAX_POW;
  p->scale_factor_max = 2.0;
  p->scale_factor_min = 1. / p->scale_factor_max;
  rt_holder_init(p, num_channels, frame_size, buffer_size, overlap_factor,
                 pad_factor, sample_rate);
  p->sample_rate        = sample_rate;
  p->num_chans          = num_channels;
  p->manip_multichannel = 0; /* implement multichannel manip later plz */
  p->phase_modif        = 1.0;

  rt_set_params(p, 1);

  p->chans = malloc(p->num_chans * sizeof(rt_chan));
  rt_uint i;
  for (i = 1; i < RT_MANIP_TYPE_COUNT; i++) {
  }
  for (i = 0; i < p->num_chans; i++) {
    p->chans[i] = rt_chan_init(p);
  }
  p->initialized = 1;
  return p;
}

rt_params rt_clean(rt_params p)
{
  rt_uint i;
  for (i = 0; i < p->num_chans; i++) {
    rt_chan_clean(p, p->chans[i]);
  }
  free(p->chans);
  free(p->hold);
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