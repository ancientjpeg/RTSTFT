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
 * @param num_frames The number of frames the framebuffer should be able to
 * hold. Usually 2.
 * @return rt_framebuf
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
rt_framebuf rt_framebuf_init(rt_params p, rt_uint num_frames)
{
  rt_framebuf framebuf       = (rt_framebuf)malloc(sizeof(rt_framebuf_t));
  framebuf->next_unread      = 0;
  framebuf->next_unprocessed = 0;
  framebuf->next_write       = 0;
  framebuf->num_frames       = num_frames;
  framebuf->phi_prev   = (rt_real *)calloc(p->fft_size / 2, sizeof(rt_real));
  framebuf->phi_cuml   = (rt_real *)calloc(p->fft_size / 2, sizeof(rt_real));
  framebuf->frame_data = (char *)calloc(framebuf->num_frames, sizeof(char));
  framebuf->omega = (rt_real *)malloc(sizeof(rt_real) * (p->fft_size / 2 + 1));
  rt_uint i;
  for (i = 0; i <= p->fft_size / 2; i++) {
    framebuf->omega[i] = /* freqs are in rads/hop */
        ((rt_real)i / p->fft_size) * 2 * M_PI * p->hop_a;
  }

  rt_uint N_bytes = p->frame_size * sizeof(rt_real);
  framebuf->frames =
      (rt_real **)malloc(sizeof(rt_real *) * framebuf->num_frames);
  framebuf->frames[0] =
      (rt_real *)pffft_aligned_malloc(N_bytes * framebuf->num_frames);
  framebuf->work = (rt_real *)pffft_aligned_malloc(N_bytes);
  for (i = 1; i < framebuf->num_frames; i++) {
    framebuf->frames[i] = framebuf->frames[0] + (p->fft_size * i);
  }

  rt_uint num_setups = p->fft_max_pow - p->fft_min_pow + 1;
  framebuf->setups = (PFFFT_Setup **)malloc(num_setups * sizeof(PFFFT_Setup *));
  rt_uint N, curr;
  for (i = p->fft_min_pow; i <= p->fft_max_pow; i++) {
    N                      = (1 << i);
    curr                   = i - p->fft_min_pow;
    framebuf->setups[curr] = pffft_new_setup(N, PFFFT_REAL);
  }

  /** pffft work array */

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
  pffft_aligned_free(framebuf->work);
  pffft_aligned_free(framebuf->frames[0]);
  rt_uint i;
  for (i = p->fft_min_pow; i <= p->fft_max_pow; i++) {
    rt_uint curr = i - p->fft_min_pow;
    pffft_destroy_setup(framebuf->setups[curr]);
  }
  free(framebuf->frames);
  free(framebuf->frame_data);
  free(framebuf->phi_prev);
  free(framebuf->phi_cuml);
  free(framebuf->omega);
  free(framebuf);
  return NULL;
}

void rt_framebuf_convert_frame(rt_params p, rt_chan c, rt_uint frame)
{
  if (!(c->framebuf->frame_data[frame] & RT_FRAME_IS_FILLED)) {
    fprintf(stderr, "Can't convert a frame that isn't filled!\n");
  }
  rt_window(c->framebuf->frames[frame] + p->pad_offset, p->frame_size);
  pffft_transform_ordered(
      c->framebuf->setups[rt_setup], c->framebuf->frames[frame],
      c->framebuf->frames[frame], c->framebuf->work, PFFFT_FORWARD);
  rt_real real, imag;
  rt_uint i;
  for (i = 2; i < p->fft_size; i += 2) {
    real                              = c->framebuf->frames[frame][i];
    imag                              = c->framebuf->frames[frame][i + 1];

    c->framebuf->frames[frame][i]     = sqrt(real * real + imag * imag);
    c->framebuf->frames[frame][i + 1] = atan2(imag, real);
  }
  c->framebuf->frame_data[frame] |= RT_FRAME_IS_CONVERTED;
}

#define wrap(a) (fmod(((a) + M_PI), 2. * M_PI) - M_PI)
void rt_framebuf_process_frame(rt_params p, rt_chan c, rt_uint frame)
{
  rt_uint i, last_frame = rt_framebuf_relative_frame(c->framebuf, frame, -1);
  if (!(c->framebuf->frame_data[last_frame] & RT_FRAME_IS_PROCESSED) &&
      !c->first_frame) {
    fprintf(stderr, "Can't process frame before the previous frame has been "
                    "processed!\n");
    exit(1);
  }
  if (p->manip_settings) {
    rt_chan input_chan = p->manip_multichannel ? c : p->chans[0];
    rt_manip_process(p, input_chan, c->framebuf->frames[frame]);
  }

  for (i = 2; i < p->fft_size; i += 2) {
    rt_real *phase_prev     = c->framebuf->phi_prev + i;
    rt_real *phase_cuml     = c->framebuf->phi_cuml + i;
    rt_real *curr_phase_ptr = c->framebuf->frames[frame] + i + 1;

    rt_real  freq_dev_wrapped =
        wrap(*curr_phase_ptr - *phase_prev - c->framebuf->omega[i]);
    rt_real freq_true = freq_dev_wrapped + c->framebuf->omega[i];

    *phase_prev       = *curr_phase_ptr;
    // *curr_phase_ptr   = wrap(*phase_cuml + (freq_true * p->scale_factor));
    *phase_cuml = *curr_phase_ptr;
  }
  c->first_frame = 0;
  c->framebuf->frame_data[frame] |= RT_FRAME_IS_PROCESSED;
  c->framebuf->next_unprocessed =
      rt_framebuf_relative_frame(c->framebuf, frame, 1);
}

void rt_framebuf_revert_frame(rt_params p, rt_chan c, rt_uint frame)
{
  rt_uint next_frame = rt_framebuf_relative_frame(c->framebuf, frame, 1);
  if (!(c->framebuf->frame_data[next_frame] & RT_FRAME_IS_PROCESSED)) {
    fprintf(stderr,
            "Can't revert frame before the next frame has been processed!\n");
    exit(1);
  }
  rt_real amp, phase;
  rt_uint i;
  for (i = 2; i < p->fft_size; i += 2) {
    amp                               = c->framebuf->frames[frame][i];
    phase                             = c->framebuf->frames[frame][i + 1];

    c->framebuf->frames[frame][i]     = amp * cos(phase);
    c->framebuf->frames[frame][i + 1] = amp * sin(phase);
  }

  pffft_transform_ordered(
      c->framebuf->setups[rt_setup], c->framebuf->frames[frame],
      c->framebuf->frames[frame], c->framebuf->work, PFFFT_BACKWARD);
  for (i = 0; i < p->fft_size; i++) {
    c->framebuf->frames[frame][i] /= (rt_real)p->fft_size;
  }
  rt_window(c->framebuf->frames[frame] + p->pad_offset, p->frame_size);
  c->framebuf->frame_data[frame] |= RT_FRAME_IS_INVERTED;
}

rt_uint rt_framebuf_relative_frame(rt_framebuf framebuf, rt_uint frame,
                                   int offset)
{
  rt_uint offset_abs = abs(offset);
  if (offset_abs > framebuf->num_frames - 1) {
    fprintf(stderr,
            "Offset cannot be of greater magnitude than num_frames - 1\n");
    return frame;
  }
  rt_uint pre_sum;
  if (offset < 0) {
    pre_sum = offset_abs > frame ? framebuf->num_frames - (offset_abs - frame)
                                 : frame - offset_abs;
  }
  else {
    pre_sum = (frame + offset) % framebuf->num_frames;
  }
  return pre_sum;
}