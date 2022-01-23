#include "rtstft.h"

rt_block rt_block_init(rt_params p)
{
  rt_block block              = (rt_block)malloc(sizeof(rt_block_t));
  block->next_unread          = 0;
  block->next_unprocessed     = 0;
  block->next_write           = 0;
  block->ready_for_processing = 0;
  block->frames = (rt_real **)malloc(sizeof(rt_real *) * p->num_frames);
  block->frames[0] =
      (rt_real *)fftw_alloc_real((size_t)p->frame_size * p->num_frames);
  for (int i = 1; i < p->num_frames; i++) {
    block->frames[i] = block->frames[0] + (p->frame_size * i);
  }
  block->frame_data = calloc(p->num_frames, sizeof(char));

  return block;
}
rt_block rt_block_destroy(rt_block block)
{
  fftw_free(block->frames[0]);
  free(block->frames);
  free(block);
  return (rt_block)NULL;
}

int rt_block_relative_frame(int num_frames, int frame, int offset)
{
  if (abs(offset) > num_frames - 1) {
    fprintf(stderr, "Offset cannot be greater than num_frames - 1\n");
    return frame;
  }
  int prelim = (frame + offset) % num_frames;
  return prelim < 0 ? num_frames - prelim : prelim;
}