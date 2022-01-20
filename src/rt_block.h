#ifndef RT_BLOCK_H
#define RT_BLOCK_H

#include "rt_globals.h"

typedef struct RT_BLOCK {
  rt_real **frames;
  int       next;
  size_t    size;
} rt_block_t;

typedef rt_block_t *rt_block;
rt_block            rt_block_init(int frame_size, int num_frames);
rt_block            rt_block_destroy(rt_block block);

#endif