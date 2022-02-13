/**
 * @file rtstft.h
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RT_PHASEVOC_H
#define RT_PHASEVOC_H

#include "rt_fifo.h"
#include "rt_framebuf.h"
#include "rt_globals.h"

/*
  - scale_factor is how much the STFT is scaling
    - i.e. 2. for and octave up shift, 1.0595 for 1 st etc.
*/

#define rt_setup (p->fft_size_pow - p->fft_min_pow)

typedef enum RTSTFT_Manipulation_Values {
  RT_MANIP_LEVEL,
  RT_MANIP_GATE,
  RT_MANIP_LIMIT,
  RT_MANIP_TYPE_COUNT
} rt_manip;

typedef struct RTSTFT_Channel {
  rt_framebuf   framebuf;
  rt_fifo       in, pre_lerp, out;
  rt_real      *manips;
  unsigned char first_frame;
} rt_chan_t;
typedef rt_chan_t *rt_chan;

/**
 * @brief The internal struct that rt_params represents.
 *
 */
typedef struct RTSTFT_Params {
  rt_uint num_chans, fft_size, fft_min_pow, fft_max_pow, fft_max_size,
      fft_size_pow, frame_size, overlap_factor, pad_factor, pad_offset, hop_a,
      hop_s, buffer_size;
  rt_real scale_factor, scale_factor_max, scale_factor_min, phase_modif,
      sample_rate;
  rt_chan      *chans;
  unsigned char manip_settings, manip_multichannel;
} rt_params_t;
typedef rt_params_t *rt_params;

/* ========  internal API  ======== */
void rt_set_params(rt_params p, rt_uint frame_size_pow, rt_uint buffer_size_pow,
                   rt_uint overlap_factor, rt_uint pad_factor,
                   rt_real scale_factor, char init);
rt_chan rt_chan_init(rt_params p);
rt_chan rt_chan_clean(rt_params p, rt_chan chan);
/* ========      API       ======== */
rt_params rt_init(rt_uint num_channels, rt_uint frame_size_pow,
                  rt_uint buffer_size_pow, rt_uint overlap_factor,
                  rt_uint pad_factor, float sample_rate);
void      rt_cycle(rt_params p, rt_real **buffers, rt_uint num_buffers,
                   rt_uint buffer_len);
void      rt_cycle_single(rt_params p, rt_real *buffer, rt_uint buffer_len);
void      rt_cycle_offset(rt_params p, rt_real **buffers, rt_uint num_buffers,
                          rt_uint buffer_len, rt_uint sample_offset);
void      rt_cycle_chan(rt_params p, rt_uint channel_index, rt_real *buffer,
                        rt_uint buffer_len);
rt_params rt_clean(rt_params p);

/* ========   rt_manip    ======== */
rt_real *rt_manip_init(rt_params p, rt_chan c);
void     rt_manip_process(rt_params p, rt_chan c, rt_real *frame_ptr);

/* ========  rt_framebuf  ======== */
rt_framebuf rt_framebuf_init(rt_params p, rt_uint num_frames);
rt_framebuf rt_framebuf_destroy(rt_params p, rt_framebuf framebuf);
rt_uint     rt_framebuf_relative_frame(rt_framebuf framebuf, rt_uint frame,
                                       int offset);
void        rt_framebuf_digest_frame(rt_params p, rt_chan c, rt_uint frame);

/* ======== MATH UTILS ======== */
void rt_hanning(rt_real *data, rt_uint len);
void rt_hamming(rt_real *data, rt_uint len);
#define rt_window rt_hanning

/* ======== MISC UTILS ======== */
rt_uint rt_real_size();
#endif