#include "rtstft.h"

rt_framebuf rt_framebuf_init(rt_params p, rt_uint num_frames)
{
  rt_framebuf framebuf       = (rt_framebuf)malloc(sizeof(rt_framebuf_t));
  framebuf->next_unread      = 0;
  framebuf->next_unprocessed = 0;
  framebuf->next_write       = 0;
  framebuf->num_frames       = num_frames;
  framebuf->last_phases      = calloc(p->frame_size / 2, sizeof(rt_real));
  framebuf->frame_data       = calloc(framebuf->num_frames, sizeof(char));
  framebuf->orig_freqs =
      (rt_real *)malloc(sizeof(rt_real) * (p->frame_size / 2 + 1));
  framebuf->frames =
      (rt_real **)malloc(sizeof(rt_real *) * framebuf->num_frames);
  /*
    Please note here: for some reason, fftw_alloc real seems to be a little iffy
    when it's placed in between the other mallocs and callocs. This error
    doesn't appear when only using one rt_params instance, and doesn't appear
    when fftw_alloc_real is placed at the end of this chain of allocs inside
    rt_framebuf_init. My suspicion is that the compiler decided to run two
    instances of rt_init somewhat "simultaneously", so that the rt_framebuf_init
    for channels 1 and 2 are run VERY close to each other. this may mess up
    something about how malloc keep track of memory, because the fftw malloc
    enforces alignment so strictly and possibly leaves extra memory at the edges
    of its allocation? Or I just don't understand at all how malloc works.
  */
  framebuf->frames[0] =
      (rt_real *)fftw_alloc_real(p->frame_size * framebuf->num_frames);
  rt_uint i;
  for (i = 1; i < framebuf->num_frames; i++) {
    framebuf->frames[i] = framebuf->frames[0] + (p->frame_size * i);
  }
  for (i = 1; i <= p->frame_size / 2; i++) {
    framebuf->orig_freqs[i] = (rt_real)i * p->sample_rate / p->frame_size;
  }

  return framebuf;
}
rt_framebuf rt_framebuf_destroy(rt_framebuf framebuf)
{
  fftw_free(framebuf->frames[0]);
  free(framebuf->frames);
  free(framebuf->frame_data);
  free(framebuf->last_phases);
  free(framebuf);
  return NULL;
}

void rt_framebuf_convert_frame(rt_params p, rt_uint frame)
{
  if (!(p->framebuf->frame_data[frame] & RT_FRAME_IS_FILLED)) {
    fprintf(stderr, "Can't convert a frame that isn't filled!\n");
  }
  rt_hanning(p->framebuf->frames[frame], p->frame_size);
  fftw_execute_r2r(p->plan, p->framebuf->frames[frame],
                   p->framebuf->frames[frame]);
  rt_real real, imag;
  rt_uint i;
  for (i = 1; i < p->frame_size / 2 - 1; i++) {
    real = p->framebuf->frames[frame][i];
    imag = p->framebuf->frames[frame][p->frame_size - i];

    p->framebuf->frames[frame][i] = sqrt(real * real + imag * imag);
    p->framebuf->frames[frame][p->frame_size - i] = atan2(imag, real);
  }
  p->framebuf->frame_data[frame] |= RT_FRAME_IS_CONVERTED;
}

#define wrap(a) (fmod(((a) + M_PI), M_PI * 2.) - M_PI)
void rt_framebuf_process_frame(rt_params p, rt_uint frame)
{

  rt_uint i, last_frame = rt_framebuf_relative_frame(p->framebuf, frame, -1);
  if (!(p->framebuf->frame_data[last_frame] & RT_FRAME_IS_PROCESSED)) {
    if (p->first_frame) {
      p->first_frame = 0;
      // for (i = 1; i < p->frame_size / 2 - 1; i++) {
      //   p->framebuf->last_phases[i] =
      //       p->framebuf->frames[frame][p->frame_size - i];
      // }
    }
    else {
      fprintf(stderr, "Can't process frame before the previous frame has been "
                      "processed!\n");
      exit(1);
    }
  }
  rt_real hop_delta   = p->hop_a / p->sample_rate;
  rt_real hop_s_delta = p->hop_s / p->sample_rate;
  for (i = 1; i < p->frame_size / 2 - 1; i++) {
    rt_uint  phase_index     = p->frame_size - i;
    rt_real  prev_phase      = p->framebuf->last_phases[i];
    rt_real  prev_phase_adj  = p->framebuf->frames[last_frame][phase_index];
    rt_real *curr_phase_ptr  = p->framebuf->frames[frame] + phase_index;
    rt_real freq_dev_wrapped = wrap((*curr_phase_ptr - prev_phase) / hop_delta);
    rt_real freq_true        = freq_dev_wrapped + p->framebuf->orig_freqs[i];
    p->framebuf->last_phases[i] = *curr_phase_ptr;
    *curr_phase_ptr             = prev_phase_adj + hop_s_delta * freq_true;
  }
  p->framebuf->frame_data[frame] |= RT_FRAME_IS_PROCESSED;
  p->framebuf->next_unprocessed =
      rt_framebuf_relative_frame(p->framebuf, frame, 1);
}

void rt_framebuf_revert_frame(rt_params p, rt_uint frame)
{
  rt_uint next_frame = rt_framebuf_relative_frame(p->framebuf, frame, 1);
  if (!(p->framebuf->frame_data[next_frame] & RT_FRAME_IS_PROCESSED)) {
    fprintf(stderr,
            "Can't revert frame before the next frame has been processed!\n");
    exit(1);
  }
  rt_real amp, phase;
  p->framebuf->frames[frame][p->frame_size / 2] = 0.;
  rt_uint i;
  for (i = 1; i < p->frame_size / 2 - 1; i++) {
    amp   = p->framebuf->frames[frame][i];
    phase = p->framebuf->frames[frame][p->frame_size - i];

    p->framebuf->frames[frame][i]                 = amp * cos(phase);
    p->framebuf->frames[frame][p->frame_size - i] = amp * sin(phase);
  }
  fftw_execute_r2r(p->plan_inv, p->framebuf->frames[frame],
                   p->framebuf->frames[frame]);
  for (i = 0; i < p->frame_size; i++) {
    p->framebuf->frames[frame][i] /= (rt_real)p->frame_size;
  }
  rt_hanning(p->framebuf->frames[frame], p->frame_size);
  p->framebuf->frame_data[frame] |= RT_FRAME_IS_INVERTED;
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