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

#define RT_BUFFER_CHANGED ((RU(1)) << 0)
#define RT_OVERLAP_CHANGED ((RU(1)) << 1)
#define RT_SAMPLERATE_CHANGED ((RU(1)) << 2)
#define RT_MANIPS_CHANGED ((RU(1)) << 3)
#define RT_PHASE_PARAMS_CHANGED ((RU(1)) << 4)
#define RT_MULTICHANNEL_CHANGED ((RU(1)) << 5)
#define RT_NUM_PARAMS_TRACKED (6)

/* see rt_parser.h for manip_flavor define */
typedef struct RTSTFT_Manip {
  rt_uint  manip_tracker, current_num_manips, manip_lock;
  rt_real *manips, *hold_manips;
} rt_manip_t;
typedef rt_manip_t *rt_manip;

typedef struct RTSTFT_Holder {
  rt_uint frame_size, buffer_size, overlap_factor, pad_factor, fft_size, setup,
      tracker, manip_multichannel;
  rt_real scale_factor, sample_rate;
  rt_real retention_mod, phase_mod, phase_chaos, gain_mod, gate_mod, limit_mod,
      dry_wet;
} rt_holder_t;
typedef rt_holder_t *rt_holder;

typedef struct RTSTFT_Channel {
  rt_framebuf framebuf;
  rt_fifo     in, out, dry;
  rt_manip    manip;
  rt_uint     frames_ready;
} rt_chan_t;
typedef rt_chan_t *rt_chan;

/**
 * @brief The internal struct that rt_params represents.
 *
 */
typedef struct RTSTFT_Params {
  rt_uint num_chans, fft_size, fft_min, fft_max, frame_size, overlap_factor,
      pad_factor, pad_offset, hop_a, hop_s, buffer_size, setup;
  rt_real scale_factor, scale_factor_max, scale_factor_min, sample_rate;
  rt_real retention_mod, phase_mod, phase_chaos, gain_mod, gate_mod, limit_mod,
      dry_wet;
  rt_chan      *chans;
  rt_listener_t listener;
  rt_holder     hold;
  rt_parser_t   parser;
  rt_uint enabled_manips, cycle_lock, samples_ingested, manip_multichannel,
      initialized;
} rt_params_t;
typedef rt_params_t *rt_params;

#endif
