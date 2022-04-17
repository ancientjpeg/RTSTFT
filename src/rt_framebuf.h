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
#define RT_FRAME_IS_PROCESSED (1 << 1)

typedef struct RTSTFT_Framebuffer {
  rt_real *frame, *omega, *phi_a_prev, *delta_phi_prev, *phi_s_cuml, *work,
      *window;
  PFFFT_Setup **setups;
} rt_framebuf_t;

typedef rt_framebuf_t *rt_framebuf;

#endif