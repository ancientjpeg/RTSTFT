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

#define rt_max(a, b) ((a) > (b) ? (a) : (b))
#define rt_min(a, b) ((a) < (b) ? (a) : (b))

rt_params rt_init(rt_uint num_channels, rt_uint frame_size_pow,
                  rt_uint buffer_size_pow, rt_uint overlap_factor,
                  rt_uint pad_factor, float sample_rate);
rt_params rt_clean(rt_params p);
void rt_set_params(rt_params p, rt_uint frame_size_pow, rt_uint buffer_size_pow,
                   rt_uint overlap_factor, rt_uint pad_factor,
                   rt_real scale_factor, char init);
void rt_cycle(rt_params p, rt_real **buffers, rt_uint num_buffers,
              rt_uint buffer_len);
void rt_cycle_single(rt_params p, rt_real *buffer, rt_uint buffer_len);
void rt_cycle_offset(rt_params p, rt_real **buffers, rt_uint num_buffers,
                     rt_uint buffer_len, rt_uint sample_offset);
void rt_cycle_chan(rt_params p, rt_uint channel_index, rt_real *buffer,
                   rt_uint buffer_len);

#endif