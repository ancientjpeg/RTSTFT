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
 * phi_prev is the analyzed frequencies from the previous frame. phi_cuml is the
 * current, calculated phase offset at each bin-this parameter is incremented by
 * the current calculated phase offset, and then wrapped, for each frame.
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
  rt_uint num_real_bins = p->fft_size / 2 + 1;
  framebuf->phi_prev    = (rt_real *)calloc(num_real_bins, sizeof(rt_real));
  framebuf->phi_cuml    = (rt_real *)calloc(num_real_bins, sizeof(rt_real));

  /**< represents per-bin phase offset in rads/hop */
  framebuf->omega = (rt_real *)malloc(sizeof(rt_real) * (num_real_bins));
  rt_uint i;
  for (i = 0; i < num_real_bins; i++) {
    framebuf->omega[i] = ((rt_real)i / p->fft_size) * 2 * M_PI * p->hop_a;
  }

  /**
   * @brief Frame allocation occurs here. Frames are allocated to maximum size
   * to prevent any need for reallocation during processing in the event of a
   * modification to the FFT size.
   */
  framebuf->frame =
      (rt_real *)pffft_aligned_malloc(2 * p->fft_max * sizeof(rt_real));
  framebuf->work = framebuf->frame + p->fft_max;

  /**< setup allocation */
  rt_uint num_setups = p->fft_max - p->fft_min + 1;
  framebuf->setups = (PFFFT_Setup **)malloc(num_setups * sizeof(PFFFT_Setup *));
  rt_uint N, curr;
  for (i = RT_FFT_MIN_POW; i <= RT_FFT_MAX_POW; i++) {
    N                      = (1 << i);
    curr                   = i - RT_FFT_MIN_POW;
    framebuf->setups[curr] = pffft_new_setup(N, PFFFT_REAL);
  }

  return framebuf;
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
  free(framebuf->phi_prev);
  free(framebuf->phi_cuml);
  free(framebuf->omega);
  free(framebuf);
  return NULL;
}

#define wrap(phi) ((phi) - (round((phi)*M_1_PI * 0.5) * 2. * M_PI))
void rt_framebuf_digest_frame(rt_params p, rt_chan c)
{
  /** variable declarations */
  rt_real *frame_ptr = c->framebuf->frame;
  rt_uint  i;
  rt_real  real, imag, amp, phase;
  rt_real  freq_dev, freq_dev_wrapped, freq_true, phase_adj;
  rt_real *phase_prev, *phase_cuml, *curr_phase_ptr;

  /** forward transform */
  rt_window(frame_ptr + p->pad_offset, p->frame_size);
  pffft_transform_ordered(c->framebuf->setups[p->setup], frame_ptr, frame_ptr,
                          c->framebuf->work, PFFFT_FORWARD);
  for (i = 2; i < p->fft_size; i += 2) {
    real             = frame_ptr[i];
    imag             = frame_ptr[i + 1];
    frame_ptr[i]     = sqrt(real * real + imag * imag);
    frame_ptr[i + 1] = atan2(imag, real);
  }

  /** manipulate */
  if (p->enabled_manips) {
    rt_chan input_chan = p->manip_multichannel ? c : p->chans[0];
    rt_manip_process(p, input_chan, frame_ptr);
  }

  /** phase adjustment */
  rt_uint frame_phase_index = 3;
  for (i = 1; i < p->fft_size / 2; i++) {
    phase_prev       = c->framebuf->phi_prev + i;
    phase_cuml       = c->framebuf->phi_cuml + i;
    curr_phase_ptr   = frame_ptr + (frame_phase_index);

    freq_dev         = *curr_phase_ptr - *phase_prev - c->framebuf->omega[i];
    freq_dev_wrapped = wrap(freq_dev);
    freq_true        = freq_dev_wrapped + c->framebuf->omega[i];
    phase_adj   = *phase_cuml + (freq_true * p->scale_factor * p->phase_modif);

    *phase_prev = *curr_phase_ptr;
    *curr_phase_ptr = wrap(phase_adj);
    *phase_cuml     = *curr_phase_ptr;
    frame_phase_index += 2;
  }

  /** begin inversion */
  for (i = 2; i < p->fft_size; i += 2) {
    amp              = frame_ptr[i];
    phase            = frame_ptr[i + 1];
    frame_ptr[i]     = amp * cos(phase);
    frame_ptr[i + 1] = amp * sin(phase);
  }
  pffft_transform_ordered(c->framebuf->setups[p->setup], frame_ptr, frame_ptr,
                          c->framebuf->work, PFFFT_BACKWARD);
  for (i = 0; i < p->fft_size; i++) {
    frame_ptr[i] /= (rt_real)p->fft_size * p->scale_factor * 4.;
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
  rt_window(frame_ptr, p->frame_size);

  /** immediately lerp to c->out */
}
