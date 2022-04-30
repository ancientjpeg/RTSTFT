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
  rt_chan chan          = (rt_chan)malloc(sizeof(rt_chan_t));
  rt_uint fifo_capacity = p->fft_max * 2;
  chan->in              = rt_fifo_init(fifo_capacity);
  chan->dry             = rt_fifo_init(fifo_capacity);
  chan->out             = rt_fifo_init(fifo_capacity);
  chan->manip           = rt_manip_init(p);
  chan->framebuf        = rt_framebuf_init(p);
  return chan;
}

rt_chan rt_chan_clean(rt_params p, rt_chan chan)
{
  chan->framebuf = rt_framebuf_destroy(p, chan->framebuf);
  chan->in       = rt_fifo_destroy(chan->in);
  chan->dry      = rt_fifo_destroy(chan->dry);
  chan->out      = rt_fifo_destroy(chan->out);
  rt_manip_clean(chan->manip);
  free(chan);
  return (rt_chan)NULL;
}