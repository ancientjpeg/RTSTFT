/**
 * @file rt_math.c
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "rtstft.h"

/* credit:
 * https://martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
 * for the fast pow approx
 */
double fastPow(double a, double b)
{
  union {
    double d;
    int    x[2];
  } u    = {a};
  u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
}

rt_real dbtoa(rt_real db_val) { return fastPow(10.f, (db_val) / 20.f); }
int     atodb(rt_real amp_val)
{
  rt_real ret = 20.f * log10f(amp_val);
  return amp_val > 0.f ? ret : INT_MIN;
}

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

void rt_lerp_samples(rt_real *in, rt_real *out, rt_uint len_I, rt_uint len_O)
{
  rt_uint len_I_adj = len_I - 1, len_O_adj = len_O - 1;
  rt_real incr      = (len_I_adj) / (len_O_adj);
  rt_real incr_curr = incr;
  rt_uint i, x0, x1;
  rt_real mod, y0, y1;
  out[0]         = in[0];
  out[len_O_adj] = in[len_I_adj];
  for (i = 1; i < len_O - 1; i++) {
    x0     = (rt_uint)incr_curr;
    x1     = x0 + 1;
    mod    = incr_curr - (rt_real)x0;
    y0     = in[x0];
    y1     = in[x1];
    out[i] = (y1 - y0) * mod + y0;
    incr_curr += incr;
  }
}
