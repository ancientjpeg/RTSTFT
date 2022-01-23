#ifndef RT_BLOCK_H
#define RT_BLOCK_H

#include "rt_globals.h"

#define RT_FRAME_IS_FILLED (1 << 0)
#define RT_FRAME_IS_TRANSFORMED (1 << 1)
#define RT_FRAME_IS_PROCESSED (1 << 2)
#define RT_FRAME_IS_INVERTED (1 << 3)

typedef struct RT_BLOCK {
  rt_real **frames;
  char     *frame_data;
  int       next_unread, next_unprocessed, next_write, ready_for_processing,
      num_frames;
  size_t size;
} rt_block_t;

typedef rt_block_t *rt_block;

#endif