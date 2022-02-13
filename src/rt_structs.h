/**
 * @file rt_structs.h
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-12
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RT_STRUCTS_H
#define RT_STRUCTS_H

#include "rt_fifo.h"
#include "rt_framebuf.h"
#include "rt_globals.h"

typedef enum RTSTFT_Manipulation_Values {
  RT_MANIP_LEVEL,
  RT_MANIP_GATE,
  RT_MANIP_LIMIT,
  RT_MANIP_TYPE_COUNT
} rt_manip;

typedef struct RTSTFT_Channel {
  rt_framebuf framebuf;
  rt_fifo     in, pre_lerp, out;
  rt_real    *manips;
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

#endif