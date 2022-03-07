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
#include "rt_parser/rt_parser.h"

#define RT_FFT_CHANGED (1UL << 0)
#define RT_BUFFER_CHANGED (1UL << 1)
#define RT_OVERLAP_CHANGED (1UL << 2)
#define RT_SCALE_CHANGED (1UL << 3)
#define RT_SAMPLERATE_CHANGED (1UL << 4)
#define RT_MANIPS_CHANGED (1UL << 5)
#define RT_NUM_PARAMS_TRACKED (6)

typedef enum RT_MANIP_TYPES {
  RT_MANIP_GAIN,
  RT_MANIP_GATE,
  RT_MANIP_LIMIT,
  RT_MANIP_TYPE_COUNT
} rt_manip_type;

typedef struct RTSTFT_Manip {
  rt_uint  manip_tracker, current_num_manips;
  rt_real *manips, *hold_manips;
} rt_manip_t;
typedef rt_manip_t *rt_manip;

typedef struct RTSTFT_Holder {
  rt_uint frame_size, buffer_size, overlap_factor, pad_factor, fft_size, setup,
      tracker;
  rt_real scale_factor, sample_rate;
} rt_holder_t;
typedef rt_holder_t *rt_holder;

typedef struct RTSTFT_Channel {
  rt_framebuf framebuf;
  rt_fifo     in, pre_lerp, out;
  rt_manip    manip;
  rt_uint     this_index;
} rt_chan_t;
typedef rt_chan_t *rt_chan;

#define RT_IN_CYCLE (1 << 0)
#define RT_AT_CYCLE_START (1 << 2)
/**
 * @brief The internal struct that rt_params represents.
 *
 */
typedef struct RTSTFT_Params {
  rt_uint num_chans, fft_size, fft_min, fft_max, frame_size, overlap_factor,
      pad_factor, pad_offset, hop_a, hop_s, buffer_size, setup;
  rt_real scale_factor, scale_factor_max, scale_factor_min, phase_modif,
      sample_rate;
  rt_chan      *chans;
  rt_holder     hold;
  rt_parser_t   parser;
  rt_uint       enabled_manips;
  unsigned char manip_multichannel, initialized, cycle_info;
} rt_params_t;
typedef rt_params_t *rt_params;

#endif