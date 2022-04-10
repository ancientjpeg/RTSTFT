/**
 * @file rt_manip.c
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "rtstft.h"

/**
 * @brief initializes the struct that holds the manipulation parameter values.
 *
 * @param p An rt_params signifying the active instance of RTSTFT.
 * @param c An rt_chan signifying the currently active channel.
 * @return rt_real*
 *
 * Internally, the sample length of all the rt_manip buffers is N / 2, as a
 * single manip buffer will only ever refer to amplitudes OR phases. These
 * internal buffers are allocated with an aligned malloc, as the FFT frames are
 * always aligned, therefore aligning the manip buffers will likely speed up
 * parameter application with SIMD instructions.
 */
rt_manip rt_manip_init(rt_params p)
{
  rt_manip m            = (rt_manip)malloc(sizeof(rt_manip_t));
  rt_uint  len          = rt_manip_block_len(p);
  m->manip_tracker      = 0;
  m->current_num_manips = p->fft_size;
  m->manips             = pffft_aligned_malloc(len * sizeof(rt_real));
  m->hold_manips        = pffft_aligned_malloc(len * sizeof(rt_real));
  rt_manip_reset(p, m);
  return m;
}

void rt_manip_clean(rt_manip m)
{
  pffft_aligned_free(m->manips);
  pffft_aligned_free(m->hold_manips);
  free(m);
}

void rt_manip_reset(rt_params p, rt_manip m)
{
  rt_uint i, j;
  for (i = 0; i < RT_MANIP_FLAVOR_COUNT; i++) {
    for (j = 0; j < rt_manip_len_max(p); j++) {
      rt_uint frame_index = rt_manip_index(p, i, j);
      switch (i) {
      case RT_MANIP_GAIN:
        m->manips[frame_index]      = 1.;
        m->hold_manips[frame_index] = 1.;
        break;
      case RT_MANIP_GATE:
        m->manips[frame_index]      = 0.;
        m->hold_manips[frame_index] = 0.;
        break;
      case RT_MANIP_LIMIT:
        m->manips[frame_index]      = 1.;
        m->hold_manips[frame_index] = 1.;
        break;
      default:
        fprintf(stderr, "Need to intialize all rt_manip fields.\n");
        exit(1);
        break;
      }
    }
  }
}

/**
 * @brief updates manips by copying values held in in hold_manips into manips
 *
 * @param p active rt_params object
 * @param c the channel for which the manips should be updated
 */
void rt_manip_update(rt_params p, rt_chan c)
{
  rt_uint  i, offset, len = rt_manip_len(p);
  rt_manip m = c->manip;
  void    *v;
  if (m->current_num_manips != p->fft_size) {
    rt_manip_framesize_changed(p, c);
  }
  else {
    for (i = 0; i < RT_MANIP_FLAVOR_COUNT; i++) {
      if (m->manip_tracker & (1UL << i)) {
        offset = rt_manip_index(p, i, 0);
        v      = memcpy(m->manips + offset, m->hold_manips + offset,
                        len * sizeof(rt_real));
      }
    }
  }
  m->manip_tracker = 0;
}

/**
 * @brief
 *
 * @param p
 * @param c
 *
 * Big note: because of how rt_lerp_samples works, the 0 and N/2 bins will be
 * unchanged during the lerp. This is beneficial because they are somewhat
 * "special" in terms of the FFT processing.
 */
void rt_manip_framesize_changed(rt_params p, rt_chan c)
{
  rt_uint  i, manip_index;
  rt_real *in, *out;
  for (i = 0; i < RT_MANIP_FLAVOR_COUNT; i++) {
    manip_index = rt_manip_index(p, i, 0);
    in          = c->manip->manips + manip_index;
    out         = c->manip->manips + manip_index;
    rt_lerp_samples(in, out, c->manip->current_num_manips, p->fft_size);
    /* because help manips still need to mirror real manips */
    memcpy(in, out, p->fft_size * sizeof(rt_real));
  }
  c->manip->current_num_manips = p->fft_size;
}

/**
 * @brief
 *
 * @param p An rt_params signifying the active instance of RTSTFT.
 * @param c An rt_chan signifying the active channel.
 * @param frame_ptr Pointer to the frame currently being processed.
 *
 * Please recall that the amplitudes come to us pre-adjusted, 0-1, from
 * framebuf.
 *
 */
void rt_manip_process(rt_params p, rt_chan c, rt_real *frame_ptr)
{

  if (p->frame_size != p->fft_size) {
    fprintf(stderr,
            "Sorry! Zero-padding and bin manipulation not yet compatible.");
    exit(1);
  }
  rt_uint  manip_index, i, manip_len = rt_manip_len(p);
  rt_real *manips
      = p->manip_multichannel ? c->manip->manips : p->chans[0]->manip->manips;
  /**
   * @brief Level manip section
   *
   */
  if (p->enabled_manips & (1 << RT_MANIP_GAIN)) {

    manip_index = rt_manip_index(p, RT_MANIP_GAIN, 0);
    for (i = 0; i < manip_len - 1; i++) {
      frame_ptr[i * 2] *= fmax(manips[manip_index++] + p->gain_mod, 0.f);
    }
    frame_ptr[1] *= fmax(manips[manip_index] + p->gain_mod, 0.f); // assign N/2 bin
  }

  /**
   * @brief Gate section
   *
   */
  rt_real thresh_adj;
  if (p->enabled_manips & (1 << RT_MANIP_GATE)) {
    manip_index = rt_manip_index(p, RT_MANIP_GATE, 0);
    for (i = 0; i < manip_len - 1; i++) {
      if (fabs(frame_ptr[i * 2]) < manips[manip_index++] + p->gate_mod) {
        frame_ptr[i * 2] = 0.f;
      }
    }
    if (fabs(frame_ptr[1]) < manips[manip_index] + p->gate_mod) {
      frame_ptr[1] = 0.f; // assign N/2 bin
    }
  }

  /**
   * @brief Limit section
   *
   */
  if (p->enabled_manips & RT_MANIP_LIMIT) {
    manip_index = rt_manip_index(p, RT_MANIP_LIMIT, 0);
    for (i = 0; i < manip_len - 1; i++) {
      thresh_adj = fmax(manips[manip_index++] + p->limit_mod, 0.f);
      if (fabs(frame_ptr[i * 2]) > thresh_adj) {
        frame_ptr[i * 2]
            = copysign(thresh_adj, frame_ptr[i * 2]); // assign N/2 bin
      }
    }
    thresh_adj = fmax(manips[manip_index] + p->limit_mod, 0.f);
    if (fabs(frame_ptr[1]) > thresh_adj) {
      frame_ptr[1] = copysign(thresh_adj, frame_ptr[1]); // assign N/2 bin
    }
  }
}

/* ==================================================================== */
/* ========================     bin setting    ======================== */
/* ==================================================================== */

void rt_manip_set_bins(rt_params p, rt_chan c, rt_manip_flavor_t manip_flavor,
                       rt_uint bin0, rt_uint binN, rt_real value)
{
  if (p->manip_multichannel == 0 && c != p->chans[0]) {
    fprintf(stderr, "Cannot set stereo manips when multichannel is disabled");
    exit(1);
  }

  rt_uint bin_curr  = bin0,
          manip_pos = rt_manip_index(p, manip_flavor, bin_curr);
  do {
    c->manip->hold_manips[manip_pos++] = value;
  } while (++bin_curr <= binN);

  c->manip->manip_tracker |= (1UL << manip_flavor);
  p->hold->tracker |= RT_MANIPS_CHANGED;
}
void rt_manip_set_bin_single(rt_params p, rt_chan c,
                             rt_manip_flavor_t manip_flavor, rt_uint bin,
                             rt_real value)
{
  if (p->manip_multichannel == 0 && c != p->chans[0]) {
    fprintf(stderr, "Cannot set stereo manips when multichannel is disabled");
    exit(1);
  }
  c->manip->hold_manips[rt_manip_index(p, manip_flavor, bin)] = value;
  c->manip->manip_tracker |= (1UL << manip_flavor);
  p->hold->tracker |= RT_MANIPS_CHANGED;
}

void rt_manip_set_bins_curved(rt_params p, rt_chan c,
                              rt_manip_flavor_t manip_flavor, rt_uint bin0,
                              rt_uint binN, rt_real value0, rt_real valueN,
                              rt_real curve_pow)
{
  if (p->manip_multichannel == 0 && c != p->chans[0]) {
    fprintf(stderr, "Cannot set stereo manips when multichannel is disabled");
    exit(1);
  }

  /* curve pow should be -10 to 10 with 0. as midpoint */
  /* it will be reversed, i.e. -10 makes a flattened curve */
  
  rt_real this_curve, this_mod;
  rt_real value_diff = valueN - value0;
  curve_pow = value_diff >= 0 ? -curve_pow : curve_pow;
  curve_pow = powf(2, curve_pow);
  rt_uint bin_curr = bin0, range = binN - bin0;
  c->manip->hold_manips[rt_manip_index(p, manip_flavor, bin_curr++)] = value0;
  c->manip->hold_manips[rt_manip_index(p, manip_flavor, binN)]       = valueN;
  do {
    this_mod   = (bin_curr - bin0) / (rt_real)range;
    this_curve = powf(this_mod, curve_pow);

    c->manip->hold_manips[rt_manip_index(p, manip_flavor, bin_curr)]
    = value_diff * this_curve + value0;
  } while (++bin_curr < binN);
  c->manip->manip_tracker |= (1UL << manip_flavor);
  p->hold->tracker |= RT_MANIPS_CHANGED;
}

/* ========================        utils       ======================== */

rt_uint rt_manip_index(rt_params p, rt_manip_flavor_t manip_flavor,
                       rt_uint frame_index)
{
  return manip_flavor * rt_manip_len_max(p) + frame_index;
}

const rt_real *rt_manip_read_buffer(rt_params p, rt_chan c,
                                    rt_manip_flavor_t manip_flavor)
{
  return c->manip->hold_manips + rt_manip_index(p, manip_flavor, 0);
}

rt_uint rt_manip_len_max(rt_params p) { return p->fft_max / 2; }
rt_uint rt_manip_len(rt_params p) { return p->fft_size / 2; }
rt_uint rt_manip_block_len(rt_params p)
{
  return rt_manip_len_max(p) * RT_MANIP_FLAVOR_COUNT;
}

void rt_manip_copy_manips(rt_params p, rt_chan c, rt_real *dest, rt_uint len)
{

  rt_uint block_len = rt_manip_block_len(p);
  if (len != block_len) {
    fprintf(stderr, "improper count for manips overwrite");
    return;
  }
  memcpy(dest, c->manip->hold_manips, block_len * sizeof(rt_real));
}

void rt_manip_overwrite_manips(rt_params p, rt_chan c, rt_real *new_manips,
                               rt_uint len)
{
  rt_uint block_len = rt_manip_block_len(p);
  if (len != block_len) {
    fprintf(stderr, "improper count for manips overwrite");
    return;
  }
  memcpy(c->manip->hold_manips, new_manips, block_len * sizeof(rt_real));
  memcpy(c->manip->manips, new_manips, block_len * sizeof(rt_real));
  c->manip->manip_tracker = 0;
  p->hold->tracker &= ~RT_MANIPS_CHANGED;
}
