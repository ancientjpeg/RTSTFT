#include "rtstft.h"

rt_block rt_block_init(rt_params p, rt_uint num_frames)
{
  rt_block block              = (rt_block)malloc(sizeof(rt_block_t));
  block->next_unread          = 0;
  block->next_unprocessed     = 0;
  block->next_write           = 0;
  block->ready_for_processing = 0;
  block->num_frames           = num_frames;
  block->frames = (rt_real **)malloc(sizeof(rt_real *) * block->num_frames);
  block->frames[0] =
      (rt_real *)fftw_alloc_real(p->frame_size * block->num_frames);
  block->orig_freqs = (rt_real *)malloc(sizeof(rt_real) * (p->frame_size / 2));
  rt_uint i;
  for (i = 1; i < block->num_frames; i++) {
    block->frames[i] = block->frames[0] + (p->frame_size * i);
  }
  for (i = 1; i <= p->frame_size / 2; i++) {
    block->orig_freqs[i] = (rt_real)i * p->sample_rate / p->frame_size;
  }
  block->frame_data = calloc(block->num_frames, sizeof(char));
  return block;
}
rt_block rt_block_destroy(rt_block block)
{
  fftw_free(block->frames[0]);
  free(block->frames);
  free(block);
  return (rt_block)NULL;
}

void rt_block_convert_frame(rt_params p, rt_uint frame)
{
  if (!(p->block->frame_data[frame] & RT_FRAME_IS_TRANSFORMED)) {
    fprintf(stderr, "Can't get amps and phases for a non-transformed frame!\n");
  }
  rt_real real, imag;
  rt_uint i;
  for (i = 1; i < p->frame_size / 2 - 1; i++) {
    real                       = p->block->frames[frame][i];
    imag                       = p->block->frames[frame][p->frame_size - i];

    p->block->frames[frame][i] = sqrt(real * real + imag * imag);
    p->block->frames[frame][p->frame_size - i] = atan2(imag, real);
  }
}

#define wrap(a) (fmod(((a) + M_PI), M_PI * 2.) - M_PI)
void rt_block_process_frame(rt_params p, rt_uint frame)
{
  rt_real t_delta_samp = 1. / p->sample_rate;
  rt_real hop_delta    = p->hop_a * t_delta_samp;
  rt_real hop_s_delta  = p->hop_s * t_delta_samp;
  rt_uint last_frame = rt_block_relative_frame(p->block->num_frames, frame, -1);
  rt_uint i;
  for (i = 1; i < p->frame_size / 2 - 1; i++) {
    rt_uint  phase_pos       = p->frame_size - i;
    rt_real  prev_phase      = p->block->frames[last_frame][phase_pos];
    rt_real *curr_phase_ptr  = p->block->frames[frame] + phase_pos;
    rt_real freq_dev_wrapped = wrap((*curr_phase_ptr - prev_phase) / hop_delta);
    rt_real freq_true        = freq_dev_wrapped + p->block->orig_freqs[i];
    *curr_phase_ptr          = prev_phase * hop_s_delta;
  }
}

void rt_block_revert_frame(rt_params p, rt_uint frame)
{
  rt_uint next_frame = rt_block_relative_frame(p->block->num_frames, frame, 1);
  if (!(p->block->frame_data[next_frame] & RT_FRAME_IS_PROCESSED)) {
    fprintf(stderr,
            "Can't revert frame before the next frame has been processed!\n");
  }
  rt_real amp, phase;
  p->block->frames[frame][p->frame_size / 2] = 0.;
  rt_uint i;
  for (i = 1; i < p->frame_size / 2 - 1; i++) {
    amp                        = p->block->frames[frame][i];
    phase                      = p->block->frames[frame][p->frame_size - i];

    p->block->frames[frame][i] = amp * cos(phase);
    p->block->frames[frame][p->frame_size - i] = amp * sin(phase);
  }
}

rt_uint rt_block_relative_frame(rt_uint num_frames, rt_uint frame, int offset)
{
  rt_uint offset_abs = abs(offset);
  if (offset_abs > num_frames - 1) {
    fprintf(stderr,
            "Offset cannot be of greater magnitude than num_frames - 1\n");
    return frame;
  }
  rt_uint pre_sum;
  if (offset < 0) {
    pre_sum = offset_abs > frame ? num_frames - (offset_abs - frame)
                                 : frame - offset_abs;
  }
  else {
    pre_sum = (frame + offset) % num_frames;
  }
  return pre_sum;
}