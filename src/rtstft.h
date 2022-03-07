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
#ifndef RTSTFT_H
#define RTSTFT_H

#include "rt_internal_API.h"

/* ========     setup    ======== */
rt_params rt_init(rt_uint num_channels, rt_uint frame_size, rt_uint buffer_size,
                  rt_uint overlap_factor, rt_uint pad_factor,
                  float sample_rate);
rt_params rt_clean(rt_params p);

/* ========     cycle    ======== */
void rt_start_cycle(rt_params p);
void rt_end_cycle(rt_params p);
void rt_cycle(rt_params p, rt_real **buffers, rt_uint buffer_len);
void rt_cycle_single(rt_params p, rt_real *buffer, rt_uint buffer_len);
void rt_cycle_offset(rt_params p, rt_real **buffers, rt_uint num_buffers,
                     rt_uint buffer_len, rt_uint sample_offset);
void rt_cycle_chan(rt_params p, rt_uint channel_index, rt_real *buffer,
                   rt_uint buffer_len);

/* ========     modify    ======== */
void rt_set_frame_size(rt_params p, rt_uint frame_size);
void rt_set_buffer_size(rt_params p, rt_uint buffer_size);
void rt_set_overlap(rt_params p, rt_uint overlap_factor);
void rt_set_pad_factor(rt_params p, rt_uint pad_factor);
void rt_set_scale_factor(rt_params p, rt_real scale_factor);
int  rt_parser_split_argv(rt_parser parser, const char *arg_str);
int  rt_parse_and_execute(rt_params p, const char *arg_str);

/* ========   MISC UTILS  ======== */
rt_uint rt_check_pow_2(rt_uint num);

#endif