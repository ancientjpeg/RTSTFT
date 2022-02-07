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
 * @bug Please note here: for some reason, fftw_alloc real seems to be a
 * little iffy when it's placed in between the other mallocs and callocs. This
 * error doesn't appear when only using one rt_params instance, and doesn't
 * appear when fftw_alloc_real is placed at the end of this chain of allocs
 * inside rt_framebuf_init. My suspicion is that the compiler decided to run
 * two instances of rt_init somewhat "simultaneously", so that the
 * rt_framebuf_init for channels 1 and 2 are run VERY close to each other.
 * this may mess up something about how malloc keep track of memory, because
 * the fftw malloc enforces alignment so strictly and possibly leaves extra
 * memory at the edges of its allocation? Or I just don't understand at all
 * how malloc works. tl;dr: fftw_alloc and fftw_plan do some weird shit so
 * never rely on fftw buffers to behave like normal ones.
 */
rt_framebuf rt_framebuf_init(rt_params p, rt_uint num_frames)
{
  rt_framebuf framebuf       = (rt_framebuf)malloc(sizeof(rt_framebuf_t));
  framebuf->next_unread      = 0;
  framebuf->next_unprocessed = 0;
  framebuf->next_write       = 0;
  framebuf->num_frames       = num_frames;
  framebuf->phases_prev = (rt_real *)calloc(p->fft_size / 2, sizeof(rt_real));
  framebuf->phases_cuml = (rt_real *)calloc(p->fft_size / 2, sizeof(rt_real));
  framebuf->frame_data  = (char *)calloc(framebuf->num_frames, sizeof(char));
  framebuf->freq_calc =
      (rt_real *)malloc(sizeof(rt_real) * (p->fft_size / 2 + 1));
  rt_uint i;
  for (i = 0; i <= p->fft_size / 2; i++) {
    framebuf->freq_calc[i] = /* freqs are in rads/hop */
        ((rt_real)i / p->fft_size) * 2 * M_PI * p->hop_a;
  }
  framebuf->frames =
      (rt_real **)malloc(sizeof(rt_real *) * framebuf->num_frames);

  framebuf->frames[0] =
      (rt_real *)pffft_aligned_malloc(p->fft_size * framebuf->num_frames);
  for (i = 1; i < framebuf->num_frames; i++) {
    framebuf->frames[i] = framebuf->frames[0] + (p->fft_size * i);
  }
  /** pffft work array */
  framebuf->work = (rt_real *)pffft_aligned_malloc(p->fft_size);

  return framebuf;
}

/**
 * @brief Cleanup function for an rt_framebuf.
 *
 * @param framebuf Frame buffer to be deallocated.
 * @return rt_framebuf
 */
rt_framebuf rt_framebuf_destroy(rt_framebuf framebuf)
{
  pffft_aligned_free(framebuf->work);
  pffft_aligned_free(framebuf->frames[0]);
  free(framebuf->frames);
  free(framebuf->frame_data);
  free(framebuf->phases_prev);
  free(framebuf->phases_cuml);
  free(framebuf->freq_calc);
  free(framebuf);
  return NULL;
}

void rt_framebuf_convert_frame(rt_params p, rt_chan c, rt_uint frame)
{
  if (!(c->framebuf->frame_data[frame] & RT_FRAME_IS_FILLED)) {
    fprintf(stderr, "Can't convert a frame that isn't filled!\n");
  }
  rt_window(c->framebuf->frames[frame] + p->pad_offset, p->frame_size);
  pffft_transform_ordered(c->setups[rt_setup], c->framebuf->frames[frame],
                          c->framebuf->frames[frame], c->framebuf->work,
                          PFFFT_FORWARD);
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
    rt_uint  phase_index    = i + 1;
    rt_real *phase_prev     = c->framebuf->phases_prev + i;
    rt_real *phase_cuml     = c->framebuf->phases_cuml + i;
    rt_real *curr_phase_ptr = c->framebuf->frames[frame] + phase_index;

    rt_real  freq_dev_wrapped =
        wrap((*curr_phase_ptr - *phase_prev) - (c->framebuf->freq_calc[i]));
    rt_real freq_true = freq_dev_wrapped + c->framebuf->freq_calc[i];
    *phase_prev       = *curr_phase_ptr;
    if (c->first_frame) {
      c->first_frame = 0;
    }
    else {
      *curr_phase_ptr = wrap(*phase_cuml + (freq_true * p->scale_factor));
    }
    *phase_cuml = *curr_phase_ptr;
  }
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

  pffft_transform_ordered(c->setups[rt_setup], c->framebuf->frames[frame],
                          c->framebuf->frames[frame], c->framebuf->work,
                          PFFFT_BACKWARD);
  for (i = 0; i < p->fft_size; i++) {
    /* idk why this isn't divided by p->fft_size but whatever */
    c->framebuf->frames[frame][i] /= (rt_real)p->frame_size;
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