/**
 * @file rt_API.c
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "rtstft.h"

rt_params rt_init(rt_uint num_channels, rt_uint frame_size, rt_uint buffer_size,
                  rt_uint overlap_factor, rt_uint pad_factor, float sample_rate)
{
  rt_params p           = malloc(sizeof(rt_params_t));
  p->initialized        = 0;
  p->fft_min            = 1UL << RT_FFT_MIN_POW; /** 2 * SIMD_SZ ^ 2 */
  p->fft_max            = 1UL << RT_FFT_MAX_POW;
  p->scale_factor_max   = 2.0;
  p->scale_factor_min   = 1. / p->scale_factor_max;
  p->manip_multichannel = 0; /* implement multichannel manip later plz */
  p->num_chans          = num_channels;
  rt_holder_init(p, num_channels, frame_size, buffer_size, overlap_factor,
                 pad_factor, sample_rate);
  rt_update_fft_size(p);
  rt_update_params(p);
  p->phase_mod = 1.0;
  rt_parser_clear_buffer(&(p->parser));
  p->chans = malloc(p->num_chans * sizeof(rt_chan));
  rt_uint i;
  for (i = 0; i < p->num_chans; i++) {
    p->chans[i] = rt_chan_init(p);
  }
  p->listener         = (rt_listener_t){};
  p->samples_ingested = 0;
  p->initialized      = 1;
  return p;
}

void rt_flush(rt_params p)
{
  rt_uint i;
  for (i = 0; i < p->num_chans; i++) {
    rt_fifo_flush(p->chans[i]->in);
    rt_fifo_flush(p->chans[i]->out);
    rt_framebuf_flush(p, p->chans[i]->framebuf);
  }
  p->samples_ingested = 0;
}
rt_params rt_clean(rt_params p)
{
  rt_uint i;
  for (i = 0; i < p->num_chans; i++) {
    rt_chan_clean(p, p->chans[i]);
  }
  free(p->chans);
  rt_holder_clean(p->hold);
  free(p);
  return (rt_params)NULL;
}

/**
 * @brief The simplest interface for cycling RTSTFT over a set of input buffers.
 *
 * @param p rt_params object representing the active RTSTFT instance.
 * @param buffers Array of rt_real buffers--should have as many buffers as
 * RTSTFT has channels.
 * @param buffer_len The length of the supplied buffers.
 *
 * Do note: rt_cycle and rt_cycle_offset are the only API cycling functions that
 * will call rt_obtain_lock(), rt_release_lock(), and rt_count_samples() for
 * you. When using all other cycling functions, these must be called manually.
 */
void rt_cycle(rt_params p, rt_real **buffers, rt_uint buffer_len)
{
  rt_cycle_offset(p, buffers, p->num_chans, buffer_len, 0);
}

void rt_cycle_single(rt_params p, rt_real *buffer, rt_uint buffer_len)
{
  rt_cycle_offset(p, &buffer, 1, buffer_len, 0);
}

void rt_cycle_offset(rt_params p, rt_real **buffers, rt_uint num_buffers,
                     rt_uint buffer_len, rt_uint sample_offset)
{
  if (!rt_obtain_cycle_lock(p)) {
    return;
  }
  if (rt_count_samples(p, buffer_len) && p->hold->tracker) {
    rt_update_params(p);
  }
  rt_uint i;
  for (i = 0; i < num_buffers; i++) {
    rt_cycle_chan(p, i, buffers[i] + sample_offset, buffer_len);
  }
  rt_release_cycle_lock(p);
}

rt_uint rt_count_samples(rt_params p, rt_uint new_samples_to_count)
{
  p->samples_ingested += new_samples_to_count;
  if (p->samples_ingested > p->fft_size) {
    p->samples_ingested %= p->fft_size;
    rt_uint i;
    for (i = 0; i < p->num_chans; i++) {
      p->chans[i]->fft_ready = 1;
    }
    return RU(1);
  }
  return RU(0);
}

rt_uint rt_obtain_cycle_lock(rt_params p)
{
  return rt_obtain_lock(&p->cycle_lock, 50000, 50);
}
void rt_release_cycle_lock(rt_params p) { rt_release_lock(&p->cycle_lock); }

rt_listener_return_t rt_get_empty_listener_data()
{
  rt_listener_return_t ret = (rt_listener_return_t){
      RT_PARAM_FLAVOR_UNDEFINED, RT_MANIP_FLAVOR_UNDEFINED, RT_REAL_ERR};
  return ret;
}
