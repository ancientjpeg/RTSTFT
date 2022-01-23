#include "rtstft.h"

rt_block rt_block_init(rt_params p, int num_frames)
{
  rt_block block              = (rt_block)malloc(sizeof(rt_block_t));
  block->next_unread          = 0;
  block->next_unprocessed     = 0;
  block->next_write           = 0;
  block->ready_for_processing = 0;
  block->num_frames           = num_frames;
  block->frames = (rt_real **)malloc(sizeof(rt_real *) * block->num_frames);
  block->frames[0] =
      (rt_real *)fftw_alloc_real((size_t)p->frame_size * block->num_frames);
  for (int i = 1; i < block->num_frames; i++) {
    block->frames[i] = block->frames[0] + (p->frame_size * i);
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

void rt_block_convert_frame(rt_params p, int frame)
{
  if (!(p->block->frame_data[frame] & RT_FRAME_IS_TRANSFORMED)) {
    fprintf(stderr, "Can't get amps and phases for a non-transformed frame!\n");
  }
  rt_real real, imag;
  for (int i = 1; i < p->frame_size / 2; i++) {
    real                       = p->block->frames[frame][i];
    imag                       = p->block->frames[frame][p->frame_size - i];

    p->block->frames[frame][i] = sqrt(real * real + imag * imag);
    p->block->frames[frame][p->frame_size - i] = atan2(imag, real);
  }
}

void rt_block_process_frame(rt_params p, int frame)
{
  for (int i = 1; i < p->frame_size / 2; i++) {
  }
}

void rt_block_revert_frame(rt_params p, int frame)
{
  int next_frame = rt_block_relative_frame(p->block->num_frames, frame, 1);
  if (!(p->block->frame_data[next_frame] & RT_FRAME_IS_PROCESSED)) {
    fprintf(stderr,
            "Can't revert frame before the next frame has been processed!\n");
  }
  rt_real amp, phase;
  p->block->frames[frame][p->frame_size / 2] = 0.;
  for (int i = 1; i < p->frame_size / 2; i++) {
    amp                        = p->block->frames[frame][i];
    phase                      = p->block->frames[frame][p->frame_size - i];

    p->block->frames[frame][i] = amp * cos(phase);
    p->block->frames[frame][p->frame_size - i] = amp * sin(phase);
  }
}

int rt_block_relative_frame(int num_frames, int frame, int offset)
{
  if (abs(offset) > num_frames - 1) {
    fprintf(stderr, "Offset cannot be greater than num_frames - 1\n");
    return frame;
  }
  int prelim = (frame + offset) % num_frames;
  return prelim < 0 ? num_frames + prelim : prelim;
}