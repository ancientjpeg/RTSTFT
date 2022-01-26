#ifndef rt_framebuf_H
#define rt_framebuf_H

#include "rt_globals.h"

#define RT_FRAME_IS_FILLED (1 << 0)
#define RT_FRAME_IS_CONVERTED (1 << 1)
#define RT_FRAME_IS_PROCESSED (1 << 2)
#define RT_FRAME_IS_INVERTED (1 << 3)

typedef struct rt_framebuf {
  rt_real **frames, *orig_freqs, *last_phases;
  char     *frame_data;
  rt_uint   next_unread, next_unprocessed, next_write, num_frames, size;
} rt_framebuf_t;

typedef rt_framebuf_t *rt_framebuf;

#endif