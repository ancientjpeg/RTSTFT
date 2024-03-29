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
  rt_real     *omega, *phi_a_prev, *omega_true_prev, *phi_s_cuml, *amp_holder;
  rt_real     *frame, *work, *window;
  PFFFT_Setup *setups[RT_FFT_POW_COUNT];
} rt_framebuf_t;

typedef rt_framebuf_t *rt_framebuf;

#endif