/**
 * @file rt_framebuf.c
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
 * @brief Init function for RTSTFT's framebuffer.
 *
 *
 * @param p An rt_params signifying the active instance of RTSTFT.
 * @return rt_framebuf returns this framebuffer.
 *
 *
 * Detailed description here. For reference, omega designates the "ideal"
 * frequencies of each FFT bin, or more accurately, the calculated phase offset
 * of a certain bin after hop_a samples. This is measured in radians.
 *
 * phi_a_prev is the analyzed frequencies from the previous frame. phi_s_cuml is
 * the current, calculated phase offset at each bin-this parameter is
 * incremented by the current calculated phase offset, and then wrapped, for
 * each frame.
 *
 */
rt_framebuf rt_framebuf_init(rt_params p)
{
  rt_framebuf framebuf = (rt_framebuf)malloc(sizeof(rt_framebuf_t));

  /**
   * @brief !!including 0 bin!!, number of bins is N / 2 + 1
   * unneeded for phase vocoder, but keeping for posterity
   *
   */
  rt_uint num_real_bins    = p->fft_max / 2 + 1;
  framebuf->phi_a_prev     = (rt_real *)malloc(num_real_bins * sizeof(rt_real));
  framebuf->delta_phi_prev = (rt_real *)malloc(num_real_bins * sizeof(rt_real));
  framebuf->phi_s_cuml     = (rt_real *)malloc(num_real_bins * sizeof(rt_real));

  /**< represents per-bin phase offset in rads/hop */
  framebuf->omega = (rt_real *)malloc(num_real_bins * sizeof(rt_real));
  rt_uint i;
  for (i = 0; i < num_real_bins; i++) {
    /** get angular frequency as w = 2pi * i * hop_a / N, which simplifies to
     * 2pi * i / overlap_factor */
    framebuf->omega[i] = ((rt_real)i / p->overlap_factor) * 2 * M_PI;
  }

  /**
   * @brief Frame allocation occurs here. Frames are allocated to maximum size
   * to prevent any need for reallocation during processing in the event of a
   * modification to the FFT size.
   */
  framebuf->frame
      = (rt_real *)pffft_aligned_malloc(3 * p->fft_max * sizeof(rt_real));
  framebuf->work   = framebuf->frame + p->fft_max;
  framebuf->window = framebuf->frame + 2 * p->fft_max;

  /**< setup allocation */
  rt_uint num_setups = p->fft_max - p->fft_min + 1;
  framebuf->setups = (PFFFT_Setup **)malloc(num_setups * sizeof(PFFFT_Setup *));
  rt_uint N, curr;
  for (i = RT_FFT_MIN_POW; i <= RT_FFT_MAX_POW; i++) {
    N                      = (1 << i);
    curr                   = i - RT_FFT_MIN_POW;
    framebuf->setups[curr] = pffft_new_setup(N, PFFFT_REAL);
  }

  rt_framebuf_flush(p, framebuf);

  return framebuf;
}

void rt_framebuf_flush(rt_params p, rt_framebuf framebuf)
{
  rt_uint num_real_bins = p->fft_max / 2 + 1;
  memset(framebuf->phi_a_prev, 0, num_real_bins * sizeof(rt_real));
  memset(framebuf->delta_phi_prev, 0, num_real_bins * sizeof(rt_real));
  memset(framebuf->phi_s_cuml, 0, num_real_bins * sizeof(rt_real));
  rt_fill_window(framebuf->window, p->frame_size);
}

/**
 * @brief Cleanup function for an rt_framebuf.
 *
 * @param framebuf Frame buffer to be deallocated.
 * @return rt_framebuf
 */
rt_framebuf rt_framebuf_destroy(rt_params p, rt_framebuf framebuf)
{
  // pffft_aligned_free(framebuf->work);
  pffft_aligned_free(framebuf->frame);
  rt_uint i;
  for (i = RT_FFT_MIN_POW; i <= RT_FFT_MAX_POW; i++) {
    rt_uint curr = i - RT_FFT_MIN_POW;
    pffft_destroy_setup(framebuf->setups[curr]);
  }
  free(framebuf->phi_a_prev);
  free(framebuf->phi_s_cuml);
  free(framebuf->delta_phi_prev);
  free(framebuf->omega);
  free(framebuf);
  return NULL;
}

/**
 * @brief Apply the window stored in the framebuf (excellent candidate for
 * vectorization).
 *
 * @details Note that ignore_padding does not window the entire fft frame;
 * instead, it applies the window starting at sample 0, which is useful as this
 * will still represent the phase-aligned signal. See the comment at the end of
 * rt_framebuf_digest_frame() for more details.
 *
 * @param p
 * @param c
 */
void rt_framebuf_apply_window(rt_params p, rt_chan c, rt_uint ignore_padding)
{
  rt_uint  i;
  rt_real *frame = c->framebuf->frame;
  if (!ignore_padding) {
    frame += p->pad_offset;
  }
  for (i = 0; i < p->fft_size; i++) {
    frame[i] *= c->framebuf->window[i];
  }
}

#define wrap(phi) ((phi) - (round((phi)*M_1_PI * 0.5) * 2. * M_PI))
void rt_framebuf_digest_frame(rt_params p, rt_chan c)
{
  /** variable declarations */
  rt_real *frame_ptr = c->framebuf->frame;
  rt_uint  i;
  rt_real  real, imag, amp, phase;
  rt_real  delta_phi, phase_adj, phase_cuml_val, phase_calc_final,
      phase_chaos_curr = 1.f;
  rt_real *phi_a_prev, *delta_phi_prev, *phi_s_cuml, *curr_phase_ptr;
  rt_real  fft_log     = (rt_real)(rt_log2_floor(p->fft_size));
  rt_real  amp_adj_rev = (rt_real)(fft_log * fft_log),
          // rt_real     amp_adj_rev = p->fft_size,
      amp_adj = 1.f / amp_adj_rev;
  signed char bin0_sign, binN_2_sign;

  /** forward transform */
  rt_framebuf_apply_window(p, c, 0);
  pffft_transform_ordered(c->framebuf->setups[p->setup], frame_ptr, frame_ptr,
                          c->framebuf->work, PFFFT_FORWARD);
  /** convert complex to amps and phases*/
  bin0_sign    = signbit(frame_ptr[0]);
  binN_2_sign  = signbit(frame_ptr[1]);
  frame_ptr[0] = (bin0_sign ? -frame_ptr[0] : frame_ptr[0]) * amp_adj;
  frame_ptr[1] = (binN_2_sign ? -frame_ptr[1] : frame_ptr[1]) * amp_adj;
  for (i = 2; i < p->fft_size; i += 2) {
    real             = frame_ptr[i];
    imag             = frame_ptr[i + 1];
    frame_ptr[i]     = sqrt(real * real + imag * imag) * amp_adj;
    frame_ptr[i + 1] = atan2f(imag, real);
  }

  /** manipulate */
  if (p->enabled_manips) {
    rt_chan input_chan = p->manip_multichannel ? c : p->chans[0];
    rt_manip_process(p, input_chan, frame_ptr);
  }

  /** phase adjustment */
  rt_uint frame_phase_index = 3;
  for (i = 1; i < p->fft_size / 2; i++) {
    phi_a_prev     = c->framebuf->phi_a_prev + i;
    delta_phi_prev = c->framebuf->delta_phi_prev + i;
    phi_s_cuml     = c->framebuf->phi_s_cuml + i;
    curr_phase_ptr = frame_ptr + (frame_phase_index);

    delta_phi      = *curr_phase_ptr - *phi_a_prev - c->framebuf->omega[i];
    delta_phi      = c->framebuf->omega[i] + wrap(delta_phi);

    /** save the correct, pitch-accurate phase */
    phase_adj = *phi_s_cuml * p->retention_mod
                + *delta_phi_prev * p->scale_factor * p->phase_mod;
    if (isnan(phase_adj)) {
      phase_adj = 0;
    }
    if (p->phase_chaos > 0.f) {
      phase_adj += (rand() - (float)(RAND_MAX >> 1)) / RAND_MAX * p->phase_chaos
                   * 2 * M_PI;
    }

    *delta_phi_prev = delta_phi;
    *phi_s_cuml     = wrap(phase_adj);

    *phi_a_prev     = *curr_phase_ptr;
    *curr_phase_ptr = *phi_s_cuml;
    frame_phase_index += 2;
  }

  /** revert amps and phases to complex */
  p->hold->amp_holder[0]                   = frame_ptr[0];
  p->hold->amp_holder[p->fft_size / 2 - 1] = frame_ptr[1];
  frame_ptr[0] *= bin0_sign * amp_adj_rev;
  frame_ptr[1] *= binN_2_sign * amp_adj_rev;
  for (i = 2; i < p->fft_size; i += 2) {
    amp                        = frame_ptr[i];
    phase                      = frame_ptr[i + 1];
    p->hold->amp_holder[i / 2] = amp;
    amp *= amp_adj_rev;
    frame_ptr[i]     = amp * cos(phase);
    frame_ptr[i + 1] = amp * sin(phase);
  }
  /** inverse transform */
  pffft_transform_ordered(c->framebuf->setups[p->setup], frame_ptr, frame_ptr,
                          c->framebuf->work, PFFFT_BACKWARD);
  rt_real overlap_adj = p->overlap_factor * 0.25f;
  rt_real final_gain  = p->scale_factor / (overlap_adj * p->fft_size);
  for (i = 0; i < p->fft_size; i++) {
    frame_ptr[i] *= final_gain;
  }
  /**
   * A note for the future:
   * We window the inverse transformed time-domain signal from the START of the
   * signal. This is because the phase vocoder calculation offsets the initial
   * phase of each sinusoid in order to keep its phase angle at the START of the
   * frame consistent with what it should be given that we are offseting its
   * starting position.
   *
   * For example, let's assume we're scaling a signal with a single bin-1
   * sinusoid, with a wavelength exactly equal to the frame size, with hop size
   * N/4. For the second frame we analyze, its phase will be pi / 2. If our
   * synthesis hop size is say N/2 (pitching up an octave), then the theoretical
   * phase that the synthesis sinusoid should have will be pi. With an FFT, this
   * will be reflected by the returned time-domain signal having a bin-1
   * sinusoid that STARTS at phase pi, i.e. signal == 0 and begins decreasing.
   * As such, we should window and overlap add starting from the first sample of
   * the returned frame, not the sample that is pad_factor samples into the
   * array.
   *
   * !!I may be wrong about this!!, but I don't think I am...
   *
   */
  rt_framebuf_apply_window(p, c, 1);

  /** immediately lerp to c->out */
}
