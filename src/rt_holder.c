/**
 * @file rt_holder.c
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "rtstft.h"

void rt_holder_init(rt_params p, rt_uint num_channels, rt_uint frame_size,
                    rt_uint buffer_size, rt_uint overlap_factor,
                    rt_uint pad_factor, rt_real sample_rate)
{
  p->hold = (rt_holder)malloc(sizeof(rt_holder_t));
  rt_set_fft_size(p, frame_size, pad_factor);
  rt_set_buffer_size(p, buffer_size);
  rt_set_overlap(p, overlap_factor);
  rt_set_pad_factor(p, pad_factor);
  rt_set_scale_factor(p, 1.0);
  p->hold->tracker = 0;
  rt_uint i        = 0;
  do {
    p->hold->tracker |= 1UL << i;
  } while (++i < RT_NUM_PARAMS_TRACKED);
}

void rt_set_params(rt_params p)
{
  const rt_holder h = p->hold;

  p->frame_size     = h->frame_size;
  p->fft_size       = h->fft_size;
  p->overlap_factor = h->overlap_factor;
  p->pad_factor     = h->pad_factor;
  p->pad_offset     = (p->fft_size - p->frame_size) / 2;
  p->scale_factor   = h->scale_factor;
  p->setup          = h->setup;
  p->sample_rate    = h->sample_rate;

  p->buffer_size    = h->buffer_size;
  p->hop_a          = p->frame_size / p->overlap_factor;
  p->hop_s          = lround(p->hop_a * p->scale_factor);
  rt_uint i;
  for (i = 1; i < RT_MANIP_TYPE_COUNT; i++) {
    p->enabled_manips |=
        1 << i; /**< sets all manipulation ON, except multichannel */
  }
  p->hold->tracker = 0;
}

void rt_set_frame_size(rt_params p, rt_uint frame_size)
{
  rt_set_fft_size(p, frame_size, p->hold->pad_factor);
}

void rt_set_buffer_size(rt_params p, rt_uint buffer_size)
{
  rt_uint buffer_pow = rt_check_pow_2(buffer_size);
  if (buffer_pow == RT_UINT_FALSE) {
    fprintf(stderr, "%lu is an invalid buffer size: not a power of 2.\n",
            buffer_size);
    exit(1);
  }
  p->hold->buffer_size = buffer_size;
  p->hold->tracker |= RT_BUFFER_CHANGED;
}

void rt_set_overlap(rt_params p, rt_uint overlap_factor)
{
  if (overlap_factor > RT_OVERLAP_MAX || overlap_factor < RT_OVERLAP_MIN) {
    fprintf(stderr, "Overlap factor must be in range %d-%d. Got value %lu\n",
            RT_OVERLAP_MIN, RT_OVERLAP_MAX, overlap_factor);
    exit(1);
  }
  p->hold->overlap_factor = overlap_factor;
  p->hold->tracker |= RT_OVERLAP_CHANGED;
}

void rt_set_pad_factor(rt_params p, rt_uint pad_factor)
{
  if (pad_factor > RT_PAD_MAX) {
    fprintf(stderr, "Pad factor must less than %d. Got value %lu\n", RT_PAD_MAX,
            pad_factor);
    exit(1);
  }
  rt_set_fft_size(p, p->hold->frame_size, pad_factor);
}

void rt_set_scale_factor(rt_params p, rt_real scale_factor)
{
  if (scale_factor > p->scale_factor_max ||
      scale_factor < p->scale_factor_min) {
    fprintf(stderr, "Scale factor must be in range %.1f-%.1f. Got value %.4f\n",
            p->scale_factor_max, p->scale_factor_min, scale_factor);
    exit(1);
  }
  p->hold->scale_factor = scale_factor;
  p->hold->tracker |= RT_SCALE_CHANGED;
}

void rt_set_sample_rate(rt_params p, rt_real sample_rate)
{
  p->hold->sample_rate = sample_rate;
  p->hold->tracker |= RT_SAMPLERATE_CHANGED;
}

void rt_set_fft_size(rt_params p, rt_uint frame_size, rt_uint pad_factor)
{
  rt_uint frame_pow = rt_check_pow_2(frame_size);
  if (frame_pow == RT_UINT_FALSE) {
    fprintf(stderr, "%lu is an invalid frame size: not a power of 2.\n",
            frame_size);
    exit(1);
  }
  rt_uint fft_pow  = frame_pow + pad_factor;
  rt_uint fft_size = 1UL << fft_pow;
  if (fft_size > p->fft_max) {
    fprintf(stderr, "Exceeded maximum frame size; got value %lu.\n", fft_size);
    exit(1);
  }
  else if (fft_size < p->fft_min) {
    fprintf(stderr, "Below minimum frame size; got value %lu.\n", fft_size);
    exit(1);
  }
  p->hold->pad_factor = pad_factor;
  p->hold->frame_size = frame_size;
  p->hold->fft_size   = fft_size;
  p->hold->setup      = fft_pow - RT_FFT_MIN_POW;
  p->hold->tracker |= RT_FFT_CHANGED;
}
