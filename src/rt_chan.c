#include "rtstft.h"

rt_chan rt_chan_init(rt_params p)
{
  rt_chan chan = (rt_chan)malloc(sizeof(rt_chan_t));
  chan->in     = rt_fifo_init(rt_max(p->buffer_size, p->fft_max_size * 2));
  rt_uint padded_out = ceil(p->fft_max_size * 2 * p->scale_factor_max);
  chan->out          = rt_fifo_init(rt_max(p->buffer_size, padded_out));
  chan->manips       = rt_manip_init(p, chan);
  chan->framebuf     = rt_framebuf_init(p);
  return chan;
}

rt_chan rt_chan_clean(rt_params p, rt_chan chan)
{
  chan->framebuf = rt_framebuf_destroy(p, chan->framebuf);
  chan->in       = rt_fifo_destroy(chan->in);
  // chan->pre_lerp = rt_fifo_destroy(chan->pre_lerp);
  chan->out = rt_fifo_destroy(chan->out);
  free(chan);
  return (rt_chan)NULL;
}