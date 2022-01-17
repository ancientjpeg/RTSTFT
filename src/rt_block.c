#include "rt_block.h"

rt_block rt_block_init(int frame_size, int num_frames)
{
  rt_block block         = (rt_block)malloc(sizeof(rt_block_t));
  block->num_frames      = num_frames;
  block->first_available = 0;
  block->frames          = (rt_real **)malloc(sizeof(rt_real *) * num_frames);
  block->frames[0]       = (rt_real *)fftw_alloc_real(frame_size * num_frames);
  for (int i = 1; i < num_frames; i++) {
    block->frames[i] = block->frames[0] + frame_size * i;
  }
  return block;
}
rt_block rt_block_destroy(rt_block block)
{
  fftw_free(block->frames[0]);
  free(block->frames);
  free(block);
  return (rt_block)NULL;
}
