/**
 * @file rt_internal_API.h
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-12
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RT_INTERNAL_API_H
#define RT_INTERNAL_API_H

#include "pffft/pffft.h"
#include "rt_structs.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define rt_setup (p->fft_size_pow - p->fft_min_pow)

/* ========  internal API  ======== */
rt_chan rt_chan_init(rt_params p);
rt_chan rt_chan_clean(rt_params p, rt_chan chan);

/* ========   rt_manip    ======== */
rt_real *rt_manip_init(rt_params p, rt_chan c);
void     rt_manip_process(rt_params p, rt_chan c, rt_real *frame_ptr);

/* ========  rt_framebuf  ======== */
rt_framebuf rt_framebuf_init(rt_params p);
rt_framebuf rt_framebuf_destroy(rt_params p, rt_framebuf framebuf);
rt_uint     rt_framebuf_relative_frame(rt_framebuf framebuf, rt_uint frame,
                                       int offset);
void        rt_framebuf_digest_frame(rt_params p, rt_chan c);

/* ======== MATH UTILS ======== */
void rt_hanning(rt_real *data, rt_uint len);
void rt_hamming(rt_real *data, rt_uint len);
#define rt_window rt_hanning

/* ======== MISC UTILS ======== */
rt_uint rt_real_size();

#endif