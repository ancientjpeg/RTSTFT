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

#ifdef __cplusplus
extern "C" {
#endif

#include "rt_internal_API.h"

/* ========     setup     ======== */
rt_params rt_init(rt_uint num_channels,
                  rt_uint frame_size,
                  rt_uint buffer_size,
                  rt_uint overlap_factor,
                  rt_uint pad_factor,
                  float   sample_rate);
void      rt_flush(rt_params p);
rt_params rt_clean(rt_params p);

/* ========     cycle     ======== */
rt_uint rt_count_samples(rt_params p, rt_uint new_samples_to_count);
rt_uint rt_obtain_cycle_lock(rt_params p);
void    rt_release_cycle_lock(rt_params p);
void    rt_cycle(rt_params p, rt_real **buffers, rt_uint buffer_len);
void    rt_cycle_single(rt_params p, rt_real *buffer, rt_uint buffer_len);
void    rt_cycle_offset(rt_params p,
                        rt_real **buffers,
                        rt_uint   num_buffers,
                        rt_uint   buffer_len,
                        rt_uint   sample_offset);
void    rt_cycle_chan(rt_params p,
                      rt_uint   channel_index,
                      rt_real  *buffer,
                      rt_uint   buffer_len);

/* ========  manipulate   ======== */
void    rt_set_frame_size(rt_params p, rt_uint frame_size);
void    rt_set_buffer_size(rt_params p, rt_uint buffer_size);
void    rt_set_overlap(rt_params p, rt_uint overlap_factor);
void    rt_set_pad_factor(rt_params p, rt_uint pad_factor);
void    rt_set_sample_rate(rt_params p, rt_real sample_rate);
void    rt_set_single_param(rt_params         p,
                            rt_param_flavor_t param_flavor,
                            rt_real           new_val);
void    rt_set_fft_size(rt_params p,
                        rt_uint   frame_size,
                        rt_uint   overlap_factor,
                        rt_uint   pad_factor);
void    rt_update_fft_size(rt_params p);
void    rt_set_multichannel(rt_params p, rt_uint new_multichannel_mode);

rt_real rt_get_param_val(rt_params p, rt_param_flavor_t param_flavor);
rt_real rt_get_manip_mod_val(rt_params p, rt_manip_flavor_t manip_flavor);

rt_uint rt_manip_obtain_manip_lock(rt_manip m);
void    rt_manip_release_manip_lock(rt_manip m);
void rt_manip_copy_manips(rt_params p, rt_chan c, rt_real *dest, rt_uint len);
void rt_manip_overwrite_manips(rt_params p,
                               rt_chan   c,
                               rt_real  *new_manips,
                               rt_uint   len);
void rt_manip_set_bin_single(rt_params         p,
                             rt_chan           c,
                             rt_manip_flavor_t manip_flavor,
                             rt_uint           bin,
                             rt_real           value);

int  rt_parse_and_execute(rt_params p, const char *arg_str);

/* ========   MISC UTILS   ======== */
rt_uint rt_check_pow_2(rt_uint num);
rt_uint rt_log2_floor(rt_uint num);
rt_real rt_dbtoa(rt_real db_val);
rt_real rt_atodb(rt_real amp_val);
rt_real rt_ptocen(rt_real pitch_ratio);
rt_real rt_centop(rt_real cents);

rt_uint rt_manip_len(rt_params p);
rt_uint rt_manip_len_max(rt_params p);
rt_uint rt_manip_block_len(rt_params p);

const rt_real *
rt_manip_read_buffer(rt_params p, rt_chan c, rt_manip_flavor_t manip_flavor);
rt_listener_return_t rt_get_empty_listener_data(void);

#ifdef __cplusplus
}
#endif

#endif
