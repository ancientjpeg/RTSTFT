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

void rt_holder_init(rt_params p,
                    rt_uint   num_channels,
                    rt_uint   frame_size,
                    rt_uint   buffer_size,
                    rt_uint   overlap_factor,
                    rt_uint   pad_factor,
                    rt_real   sample_rate)
{
  p->hold = (rt_holder)malloc(sizeof(rt_holder_t));
  rt_set_single_param(p, RT_SCALE_FACTOR_MOD, 1.f);
  rt_set_single_param(p, RT_RETENTION_MOD, 1.f);
  rt_set_single_param(p, RT_PHASE_MOD, 1.f);
  rt_set_single_param(p, RT_PHASE_CHAOS, 0.f);
  rt_set_single_param(p, RT_PARAM_GATE_MOD, 1.f);
  rt_set_single_param(p, RT_PARAM_GAIN_MOD, 1.f);
  rt_set_single_param(p, RT_PARAM_LIMIT_MOD, 1.f);
  rt_set_single_param(p, RT_DRY_WET, 1.f);
  rt_set_buffer_size(p, buffer_size);
  rt_set_fft_size(p, frame_size, overlap_factor, pad_factor);
  p->hold->tracker = 0;
  rt_uint i        = 0;
  do {
    p->hold->tracker |= 1UL << i;
  } while (++i < RT_NUM_PARAMS_TRACKED);
  for (i = 0; i < RT_MANIP_FLAVOR_COUNT; i++) {
    p->enabled_manips
        |= 1 << i; /**< sets all manipulation ON, except multichannel */
  }
}

void rt_holder_clean(rt_holder hold)
{
  free(hold);
}

void rt_update_fft_size(rt_params p)
{
  if (!rt_obtain_cycle_lock(p)) {
    exit(72);
  };
  rt_uint i;
  if (p->initialized) {
    for (i = 0; i < p->num_chans; i++) {
      if (!rt_manip_obtain_manip_lock(p->chans[i]->manip)) {
        exit(72);
      }
    }
  }
  const rt_holder h = p->hold;
  p->frame_size     = h->frame_size;
  p->fft_size       = h->fft_size;
  p->overlap_factor = h->overlap_factor;
  p->pad_factor     = h->pad_factor;
  p->pad_offset     = (p->fft_size - p->frame_size) / 2;
  p->setup          = h->setup;
  p->hop_a          = p->frame_size / p->overlap_factor;

  if (p->initialized) {
    for (i = 0; i < p->num_chans; i++) {
      rt_manip_framesize_changed(p, p->chans[i]);
    }
    rt_flush(p);
    for (i = 0; i < p->num_chans;
         rt_manip_release_manip_lock(p->chans[i++]->manip))
      ;
  }
  rt_update_params(p);
  rt_release_cycle_lock(p);
}
void rt_update_manips(rt_params p)
{
  rt_uint i, num_chans = p->manip_multichannel ? p->num_chans : 1;
  for (i = 0; i < num_chans; i++) {
    rt_manip_update(p, p->chans[i]);
  }
}
/**
 * @brief A function called before every FFT block is processed to update
 * parameters, used to ensure threadsafety.
 *
 * @details This function is integral the the thread-safe functionality of
 * RTSTFT. First, rt_manips are updated; this ensures that, if the FFT size has
 * also changed, that the manips will first be written, *then* scaled.
 *
 * It is HIGHLY unlikely you will ever need to call this function manually-
 * doing so will likely result in unpredictable behavior. One example of when it
 * might be useful to do so would be disabling your host program's audio
 * processing, editing the internals of your rt_params, and then running
 * rt_update_params(p). This is useful for things like copying in whole blocks
 * of manips, which might take a bit too long to execute in a realtime audio
 * plugin.
 *
 * @param p The active rt_params instance
 */
void rt_update_params(rt_params p)
{
  const rt_holder h = p->hold;

  if ((p->hold->tracker & RT_MANIPS_CHANGED) && p->initialized) {
    rt_update_manips(p);
  }

  p->sample_rate   = h->sample_rate;
  p->buffer_size   = h->buffer_size;

  p->scale_factor  = h->scale_factor;
  p->retention_mod = h->retention_mod;
  p->phase_mod     = h->phase_mod;
  p->phase_chaos   = h->phase_chaos;
  p->gain_mod      = h->gain_mod;
  p->gate_mod      = h->gate_mod;
  p->limit_mod     = h->limit_mod;
  p->dry_wet       = h->dry_wet;

  p->hop_s         = (rt_uint)lround(p->hop_a * p->scale_factor);

  p->hold->tracker = 0;
}

/**
 * @brief THIS IS NOT THREADSAFE
 *
 * @param p
 * @param frame_size
 */
void rt_set_frame_size(rt_params p, rt_uint frame_size)
{
  rt_set_fft_size(p, frame_size, p->hold->overlap_factor, p->hold->pad_factor);
}

/**
 * @brief THIS IS NOT THREADSAFE
 *
 * @param p
 * @param frame_size
 */
void rt_set_overlap(rt_params p, rt_uint overlap_factor)
{
  if (overlap_factor > RT_OVERLAP_MAX || overlap_factor < RT_OVERLAP_MIN) {
    fprintf(stderr,
            "Overlap factor must be in range " ru "-" ru ". Got value " ru "\n",
            RT_OVERLAP_MIN, RT_OVERLAP_MAX, overlap_factor);
    exit(1);
  }
  rt_set_fft_size(p, p->hold->frame_size, overlap_factor, p->hold->pad_factor);
}

void rt_set_pad_factor(rt_params p, rt_uint pad_factor)
{
  if (pad_factor > RT_PAD_MAX) {
    fprintf(stderr, "Pad factor must less than %d. Got value " ru "\n",
            RT_PAD_MAX, pad_factor);
    exit(1);
  }
  rt_set_fft_size(p, p->hold->frame_size, p->hold->overlap_factor, pad_factor);
}

void rt_set_buffer_size(rt_params p, rt_uint buffer_size)
{
  rt_uint buffer_pow = rt_check_pow_2(buffer_size);
  if (buffer_pow == RT_UINT_FALSE) {
    fprintf(stderr, ru " is an invalid buffer size: not a power of 2.\n",
            buffer_size);
    exit(1);
  }
  p->hold->buffer_size = buffer_size;
  p->hold->tracker |= RT_BUFFER_CHANGED;
}

void rt_set_sample_rate(rt_params p, rt_real sample_rate)
{
  p->hold->sample_rate = sample_rate;
  p->hold->tracker |= RT_SAMPLERATE_CHANGED;
}

/**
 * @brief THIS IS NOT THREADSAFE
 *
 * @param p
 * @param frame_size
 */
void rt_set_fft_size(rt_params p,
                     rt_uint   frame_size,
                     rt_uint   overlap_factor,
                     rt_uint   pad_factor)
{

  p->hold->overlap_factor = overlap_factor;
  p->hold->tracker |= RT_OVERLAP_CHANGED;

  rt_uint frame_pow = rt_check_pow_2(frame_size);
  if (frame_pow == RT_UINT_FALSE) {
    fprintf(stderr, ru " is an invalid frame size: not a power of 2.\n",
            frame_size);
    exit(1);
  }
  rt_uint fft_pow  = frame_pow + pad_factor;
  rt_uint fft_size = RU(1) << fft_pow;
  if (fft_size > p->fft_max) {
    fprintf(stderr, "Exceeded maximum frame size; got value " ru ".\n",
            fft_size);
    exit(1);
  }
  else if (fft_size < p->fft_min) {
    fprintf(stderr, "Below minimum frame size; got value " ru ".\n", fft_size);
    exit(1);
  }
  p->hold->pad_factor = pad_factor;
  p->hold->frame_size = frame_size;
  p->hold->fft_size   = fft_size;
  p->hold->setup      = fft_pow - RT_FFT_MIN_POW;
}

void rt_set_single_param(rt_params         p,
                         rt_param_flavor_t param_flavor,
                         rt_real           new_val)
{
  switch (param_flavor) {
  case RT_SCALE_FACTOR_MOD:
    p->hold->scale_factor = new_val;
    break;
  case RT_RETENTION_MOD:
    p->hold->retention_mod = new_val;
    break;
  case RT_PHASE_MOD:
    p->hold->phase_mod = new_val;
    break;
  case RT_PHASE_CHAOS:
    p->hold->phase_chaos = new_val;
    break;
  case RT_PARAM_GAIN_MOD:
    p->hold->gain_mod = new_val;
    break;
  case RT_PARAM_GATE_MOD:
    p->hold->gate_mod = new_val;
    break;
  case RT_PARAM_LIMIT_MOD:
    p->hold->limit_mod = new_val;
    break;
  case RT_DRY_WET:
    p->hold->dry_wet = new_val;
    break;
  default:
    break;
  }
  p->hold->tracker |= RT_PHASE_PARAMS_CHANGED;
}

rt_real rt_get_param_val(rt_params p, rt_param_flavor_t param_flavor)
{

  switch (param_flavor) {
  case RT_SCALE_FACTOR_MOD:
    return p->hold->scale_factor;
  case RT_RETENTION_MOD:
    return p->hold->retention_mod;
  case RT_PHASE_MOD:
    return p->hold->phase_mod;
  case RT_PHASE_CHAOS:
    return p->hold->phase_chaos;
  case RT_PARAM_GAIN_MOD:
    return p->hold->gain_mod;
  case RT_PARAM_GATE_MOD:
    return p->hold->gate_mod;
  case RT_PARAM_LIMIT_MOD:
    return p->hold->limit_mod;
  default:
    return RT_REAL_ERR;
  }
}

rt_real rt_get_manip_mod_val(rt_params p, rt_manip_flavor_t manip_flavor)
{
  switch (manip_flavor) {

  case RT_MANIP_GAIN:
    return p->hold->gain_mod;
  case RT_MANIP_GATE:
    return p->hold->gate_mod;
  case RT_MANIP_LIMIT:
    return p->hold->limit_mod;
  default:
    return RT_REAL_ERR;
  }
}

void rt_on_multichannel_change(rt_params p)
{
  if (!(p->hold->tracker & RT_MULTICHANNEL_CHANGED)) {
    return;
  }
  rt_chan chan0 = p->chans[0];
  if (!rt_manip_obtain_manip_lock(chan0->manip)) {
    exit(77);
  }
  rt_uint i, c, m, m_ind;
  rt_real mean, *c0_ptr;
  if (!rt_obtain_cycle_lock(p)) {
    exit(76);
  }
  p->manip_multichannel = p->hold->manip_multichannel;
  p->hold->tracker &= ~RT_MULTICHANNEL_CHANGED;
  for (m = 0; m < RT_MANIP_FLAVOR_COUNT; m++) {
    for (i = 0; i < rt_manip_len(p); i++) {
      m_ind  = rt_manip_index(p, m, i);
      c0_ptr = chan0->manip->hold_manips + m_ind;
      if (p->manip_multichannel) {
        /* copy chan0 to all other chans */
        for (c = 1; c < p->num_chans; c++) {
          p->chans[c]->manip->hold_manips[m_ind] = *c0_ptr;
        }
      }
      else {
        /* average all chans into chan0 */
        mean = 0.;
        for (c = 0; c < p->num_chans; c++) {
          mean += p->chans[c]->manip->hold_manips[m_ind];
        }
        *c0_ptr = mean / p->num_chans;
      }
    }
  }
  p->hold->tracker |= RT_MANIPS_CHANGED;
  rt_flush(p);
  rt_release_cycle_lock(p);
  rt_manip_release_manip_lock(chan0->manip);
}

void rt_set_multichannel(rt_params p, rt_uint new_multichannel_mode)
{
  new_multichannel_mode = new_multichannel_mode > 0 ? 1 : 0;
  if (p->manip_multichannel ^ new_multichannel_mode) {
    p->hold->tracker |= RT_MULTICHANNEL_CHANGED;
  }
  p->hold->manip_multichannel = new_multichannel_mode;
  rt_on_multichannel_change(p);
}
