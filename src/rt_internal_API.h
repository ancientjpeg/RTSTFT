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

#define rt_max(a, b) ((a) > (b) ? (a) : (b))
#define rt_min(a, b) ((a) < (b) ? (a) : (b))

/* ========    rt_chan    ======== */
rt_chan rt_chan_init(rt_params p);
rt_chan rt_chan_clean(rt_params p, rt_chan chan);

/* ========   rt_holder   ======== */
void rt_holder_init(rt_params p, rt_uint num_channels, rt_uint frame_size,
                    rt_uint buffer_size, rt_uint overlap_factor,
                    rt_uint pad_factor, rt_real sample_rate);
void rt_holder_clean(rt_holder hold);
void rt_update_params(rt_params p);
void rt_update_manips(rt_params p);
void rt_params_check_mod(rt_params p);

/* ========   rt_manip    ======== */
rt_manip rt_manip_init(rt_params p);
void     rt_manip_clean(rt_manip m);
void     rt_manip_reset(rt_params p, rt_manip m);
void     rt_manip_update(rt_params p, rt_chan c);
void     rt_manip_process(rt_params p, rt_chan c, rt_real *frame_ptr);
void     rt_manip_framesize_changed(rt_params p, rt_chan c);
void rt_manip_set_bins(rt_params p, rt_chan c, rt_manip_flavor_t manip_flavor,
                       rt_uint bin0, rt_uint binN, rt_real value);
void rt_manip_set_bins_curved(rt_params p, rt_chan c,
                              rt_manip_flavor_t manip_flavor, rt_uint bin0,
                              rt_uint binN, rt_real value0, rt_real valueN,
                              rt_real curve_pow);
rt_uint rt_manip_index(rt_params p, rt_manip_flavor_t manip_flavor,
                       rt_uint frame_index);

/* ========    rt_fifo    ======== */
rt_fifo rt_fifo_init(rt_uint len);
void    rt_fifo_enqueue(rt_fifo fifo, rt_real *data, int n);
void    rt_fifo_enqueue_one(rt_fifo fifo, rt_real data);
void rt_fifo_enqueue_staggered(rt_fifo fifo, rt_real *data, int n, int advance);
void rt_fifo_read(rt_fifo fifo, rt_real *dest, int n);
void rt_fifo_dequeue(rt_fifo fifo, int n);
void rt_fifo_dequeue_one(rt_fifo fifo, rt_real *dest);
void rt_fifo_dequeue_staggered(rt_fifo fifo, rt_real *dest, int n, int advance);
rt_uint rt_fifo_payload(rt_fifo fifo);
rt_uint rt_fifo_readable(rt_fifo fifo);
void    rt_fifo_flush(rt_fifo fifo);
rt_fifo rt_fifo_destroy(rt_fifo fifo);

/* ========  rt_framebuf  ======== */
rt_framebuf rt_framebuf_init(rt_params p);
void        rt_framebuf_flush(rt_params p, rt_framebuf framebuf);
rt_framebuf rt_framebuf_destroy(rt_params p, rt_framebuf framebuf);
rt_uint     rt_framebuf_relative_frame(rt_framebuf framebuf, rt_uint frame,
                                       int offset);
void        rt_framebuf_digest_frame(rt_params p, rt_chan c);

/* ======== MATH UTILS ======== */
double fastPow(double a, double b);
void   rt_lerp_samples(rt_real *in, rt_real *out, rt_uint len_I, rt_uint len_O);
void   rt_hanning(rt_real *data, rt_uint len);
void   rt_fill_hanning(rt_real *data, rt_uint len);
void   rt_hamming(rt_real *data, rt_uint len);
void   rt_fill_hamming(rt_real *data, rt_uint len);
#define rt_window rt_hanning
#define rt_fill_window rt_fill_hanning

/* ======== MISC UTILS ======== */
// https://stackoverflow.com/questions/14818084/what-is-the-proper-include-for-the-function-sleep

#ifdef _WIN32
#include <Windows.h>
#define rt_usleep(us) Sleep((us) / 1000)
#define rt_msleep(ms) Sleep((ms))
#define rt_sleep(s) Sleep((s)*1000)
#else
#include <unistd.h>
#define rt_usleep(us) usleep((us))
#define rt_msleep(ms) usleep((ms)*1000)
#define rt_sleep(s) sleep((s))
#endif

    rt_uint
    rt_obtain_lock(rt_uint *lock, rt_uint timeout_us, rt_uint refresh_us);
void rt_release_lock(rt_uint *lock);

#endif
