/**
 * @file rt_chan.c
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "rtstft.h"

rt_chan rt_chan_init(rt_params p)
{
  rt_chan chan       = (rt_chan)malloc(sizeof(rt_chan_t));
  chan->in           = rt_fifo_init(rt_max(p->buffer_size * 2, p->fft_max * 2));
  rt_uint padded_out = ceil(p->fft_max * 2 * p->scale_factor_max);
  chan->out          = rt_fifo_init(rt_max(p->buffer_size * 2, padded_out));
  chan->manip        = rt_manip_init(p);
  chan->framebuf     = rt_framebuf_init(p);
  return chan;
}

rt_chan rt_chan_clean(rt_params p, rt_chan chan)
{
  chan->framebuf = rt_framebuf_destroy(p, chan->framebuf);
  chan->in       = rt_fifo_destroy(chan->in);
  chan->out      = rt_fifo_destroy(chan->out);
  rt_manip_clean(chan->manip);
  free(chan);
  return (rt_chan)NULL;
}