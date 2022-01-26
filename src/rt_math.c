#include "rtstft.h"

void rt_hanning(rt_real *data, rt_uint len)
{
  rt_uint n;
  for (n = 0; n < len; n++) {
    rt_real hann = 0.5 - 0.5 * cos(2. * M_PI * n / (len - 1));
    data[n] *= hann;
  }
}

void rt_hamming(rt_real *data, rt_uint len)
{
  rt_uint n;
  for (n = 0; n < len; n++) {
    rt_real hamm = 0.54 - 0.46 * cos(2. * M_PI * n / (len - 1));
    data[n] *= hamm;
  }
}

float get_fbin(int bin, rt_params p)
{
  return p->sample_rate / p->frame_size * bin;
}

/* unused — for ref */
void rt_lerp(rt_params p, rt_real *out, rt_uint out_size, rt_real *in,
             rt_uint in_size)
{
  rt_real input_incr = (rt_real)(in_size - 1) / (out_size - 1);
  out[0]             = in[0];
  rt_real input_pos;
  rt_uint i;
  for (i = 1; i < out_size - 1; i++) {
    input_pos     = i * input_incr;
    rt_uint index = (rt_uint)input_pos;
    rt_real mod   = input_pos - floor(input_pos);
    out[i]        = (in[index + 1] - in[index]) * mod + in[index];
  }
  out[out_size - 1] = in[in_size - 1];
}