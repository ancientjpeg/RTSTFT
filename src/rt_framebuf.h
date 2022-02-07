/**
 * @file rt_framebuf.h
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef rt_framebuf_H
#define rt_framebuf_H

#include "rt_globals.h"

#define RT_FRAME_IS_FILLED (1 << 0)
#define RT_FRAME_IS_CONVERTED (1 << 1)
#define RT_FRAME_IS_PROCESSED (1 << 2)
#define RT_FRAME_IS_INVERTED (1 << 3)

typedef struct rt_framebuf {
  rt_real     **frames, *omega, *phi_prev, *phi_cuml, *work;
  char         *frame_data;
  PFFFT_Setup **setups;
  rt_uint       next_unread, next_unprocessed, next_write, num_frames, size;
} rt_framebuf_t;

typedef rt_framebuf_t *rt_framebuf;

#endif