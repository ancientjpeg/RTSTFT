#include "rtstft.h"

void hanning(rt_real *data, size_t len)
{
  for (size_t n = 0; n < len; n++) {
    rt_real hann = 0.5 - 0.5 * cos(2. * M_PI * n / (len - 1));
    data[n] *= hann;
  }
}

void hamming(rt_real *data, size_t len)
{
  for (size_t n = 0; n < len; n++) {
    rt_real hamm = 0.54 - 0.46 * cos(2. * M_PI * n / (len - 1));
    data[n] *= hamm;
  }
}

float get_fbin(int bin, rt_params p)
{
  return p->sample_rate / p->frame_size * bin;
}

void rt_lerp(rt_params p, rt_real *in, size_t in_size, rt_real *out,
             size_t out_size)
{
  rt_real interp_incr = (rt_real)in_size / (out_size - 1);
  rt_real j           = interp_incr;
  out[0]              = in[0];
  for (size_t i = 1; i < out_size - 1; i++) {
    size_t  curr = (size_t)j;
    rt_real mod  = j - floor(j);
    out[i]       = (in[curr + 1] - in[curr]) * mod + in[curr];
    j += p->lerp_incr;
  }
  out[out_size - 1] = in[in_size - 1];
}