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

void rt_fill_hanning(rt_real *data, rt_uint len)
{
  rt_uint n;
  for (n = 0; n < len; n++) {
    rt_real hann = 0.5 - 0.5 * cos(2. * M_PI * n / (len - 1));
    data[n]      = hann;
  }
}

void rt_hanning(rt_real *data, rt_uint len)
{
  rt_uint n;
  for (n = 0; n < len; n++) {
    rt_real hann = 0.5 - 0.5 * cosf(2. * M_PI * n / (len - 1));
    data[n] *= hann;
  }
}

void rt_fill_hamming(rt_real *data, rt_uint len)
{
  rt_uint n;
  for (n = 0; n < len; n++) {
    rt_real hamm = 0.54 - 0.46 * cosf(2. * M_PI * n / (len - 1));
    data[n]      = hamm;
  }
}

void rt_hamming(rt_real *data, rt_uint len)
{
  rt_uint n;
  for (n = 0; n < len; n++) {
    rt_real hamm = 0.54 - 0.46 * cosf(2. * M_PI * n / (len - 1));
    data[n] *= hamm;
  }
}

void rt_lerp_samples(rt_real *in, rt_real *out, rt_uint len_I, rt_uint len_O)
{
  rt_uint len_I_adj = len_I - 1, len_O_adj = len_O - 1;
  rt_real incr      = (rt_real)(len_I_adj) / (len_O_adj);
  rt_real incr_curr = incr;
  rt_uint i, x0, x1;
  rt_real mod, y0, y1;
  out[0]         = in[0];
  out[len_O_adj] = in[len_I_adj];
  for (i = 1; i < len_O_adj; i++) {
    x0     = (rt_uint)incr_curr;
    x1     = x0 + 1;
    mod    = incr_curr - (rt_real)x0;
    y0     = in[x0];
    y1     = in[x1];
    out[i] = (y1 - y0) * mod + y0;
    incr_curr += incr;
  }
}

rt_uint rt_log2_floor(rt_uint num)
{
  int power = 0;
  while (num >> (power) > 0)
    power++;
  return power == 0 ? 0 : power - 1;
}

rt_uint rt_check_pow_2(rt_uint num)
{
  rt_uint i = 0, check;
  do {
    check = RU(1) << i;
    if (check & num) {
      if (~check & num) {
        fprintf(stderr,
                "FFT size must be a power of two. Supplied value: " ru "\n",
                num);
        return RT_UINT_FALSE;
      }
      return i;
    }
  } while (i++ < RT_FFT_MAX_POW);

  fprintf(stderr, ru " is an invalid frame size. Must be greater than " ru "\n",
          num, RU(1) << RT_FFT_MIN_POW);
  return RT_UINT_FALSE;
}

rt_real rt_dbtoa(rt_real db_val) { return powf(10.f, (db_val) / 20.f); }
rt_real rt_atodb(rt_real amp_val)
{
  rt_real ret = 20.f * log10f(amp_val);
  return amp_val > 0.f ? ret : FLT_MIN;
}

/**
 * @brief Generic function to wait for a "lock"
 *
 * @details In RTSTFT, a "lock" is just a boolean stored as an rt_uint, which is
 * "locked" whenever it is nonzero. Any call to rt_obtain_lock or any of its
 * derived functions necessitates a call to their corresponding rt_release_lock
 * function - no destructors here!
 *
 * @param lock
 * @param timeout_us
 * @param refresh_us
 * @return rt_uint
 *
 *
 */
rt_uint rt_obtain_lock(rt_uint *lock, rt_uint timeout_us, rt_uint refresh_us)
{
  if (*lock == 0) {
    *lock = 1;
    return RU(1);
  }
  rt_uint timeout_count = timeout_us / refresh_us;
  while (*lock != 0 && timeout_count-- > 0) {
    rt_usleep(refresh_us);
  }
  if (timeout_count) {
    *lock = 1;
    return RU(1);
  }
  return RU(0);
}
void rt_release_lock(rt_uint *lock) { *lock = 0; }
