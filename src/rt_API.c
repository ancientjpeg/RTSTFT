#include "rtstft.h"
#define rt_max(a, b) ((a) > (b) ? (a) : (b))
#define rt_min(a, b) ((a) < (b) ? (a) : (b))

rt_chan rt_chan_init(rt_params p)
{
  rt_chan chan       = malloc(sizeof(rt_chan_t));
  chan->framebuf     = rt_framebuf_init(p, 2);
  chan->in           = rt_fifo_init(rt_max(p->buffer_size, p->frame_size * 2));
  rt_uint lerp_frame = p->overlap_factor * p->hop_s + p->frame_size;
  chan->pre_lerp     = rt_fifo_init(
          rt_max((rt_uint)ceil((rt_real)p->buffer_size * 2 * p->scale_factor),
                 lerp_frame * 2));
  chan->out = rt_fifo_init(rt_max(p->buffer_size, p->frame_size * 2));
  chan->plan =
      fftw_plan_r2r_1d(p->fft_size, chan->framebuf->frames[0],
                       chan->framebuf->frames[0], FFTW_R2HC, FFTW_ESTIMATE);
  chan->plan_inv =
      fftw_plan_r2r_1d(p->fft_size, chan->framebuf->frames[0],
                       chan->framebuf->frames[0], FFTW_HC2R, FFTW_ESTIMATE);
  return chan;
}

rt_chan rt_chan_clean(rt_chan chan)
{
  chan->framebuf = rt_framebuf_destroy(chan->framebuf);
  chan->in       = rt_fifo_destroy(chan->in);
  chan->pre_lerp = rt_fifo_destroy(chan->pre_lerp);
  fftw_destroy_plan(chan->plan);
  fftw_destroy_plan(chan->plan_inv);
  free(chan);
  return (rt_chan)NULL;
}

rt_params rt_init(rt_uint num_channels, rt_uint frame_size,
                  rt_uint overlap_factor, rt_uint buffer_size,
                  float sample_rate, float scale_factor)
{
  if (frame_size * sizeof(float) % 16 != 0) {
    fprintf(stderr, "Frames  must be able to by byte-aligned to 16 bytes.");
    exit(1);
  }
  else if (!frame_size || overlap_factor <= 0.f) {
    fprintf(stderr, "Cannot have frame size or overlap factor be <= zero.");
    exit(1);
  }
  rt_params p     = malloc(sizeof(rt_params_t));
  p->scale_factor = scale_factor;
  p->pad_factor   = 0;
  p->frame_size   = frame_size;
  p->fft_size     = frame_size * (1 << p->pad_factor);
  p->pad_offset   = (p->fft_size - p->frame_size) / 2;
  p->frame_max    = 1 << 15;
  if (p->fft_size > p->frame_max) {
    fprintf(stderr, "Exceeded maximum frame size.\n");
    exit(1);
  }
  else if (p->fft_size < 32) {
    fprintf(stderr, "Below minimum frame size.\n");
    exit(1);
  }
  p->overlap_factor = overlap_factor;
  p->buffer_size    = buffer_size;
  p->sample_rate    = sample_rate;
  p->hop_a          = p->frame_size / p->overlap_factor;
  p->hop_s          = lround(p->hop_a * p->scale_factor);
  p->num_chans      = num_channels;
  p->chans          = malloc(p->num_chans * sizeof(rt_chan));
  rt_uint i;
  for (i = 0; i < p->num_chans; i++) {
    p->chans = rt_chan_init(p);
  }
  return p;
}

rt_params rt_clean(rt_params p)
{
  rt_uint i;
  for (i = 0; i < p->num_chans; i++) {
    rt_chan_clean(p->chans[i]);
  }
  free(p->chans);
  free(p);
  return (rt_params)NULL;
}