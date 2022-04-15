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

rt_uint rt_log2_floor(rt_uint num)
{
  int power = 0;
  while (num >> (power++) > 0)
    ;
  return power;
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

  fprintf(stderr, "%lu is an invalid frame size. Must be greater than %lu.\n",
          num, 1UL << RT_FFT_MIN_POW);
  return RT_UINT_FALSE;
}

rt_real rt_dbtoa(rt_real db_val) { return fastPow(10.f, (db_val) / 20.f); }
int     rt_atodb(rt_real amp_val)
{
  rt_real ret = 20.f * log10f(amp_val);
  return amp_val > 0.f ? ret : INT_MIN;
}

/**
 * @brief Waits for an rt_uint to become 0.
 *
 * @param val         Pointer to the value being waited on
 * @param timeout_us Timeout in microseconds
 * @param refresh_us Refresh rate in microseconds
 * @return rt_uint if 0, function returned because of a timeout. Else,
 * returned because the value became 0.
 */
rt_uint rt_await_zero_val(const rt_uint *val, rt_uint timeout_us,
                          rt_uint refresh_us)
{
  if (*val == 0) {
    return RU(1);
  }
  rt_uint timeout_count = timeout_us / refresh_us;
  while (*val != 0 && timeout_count-- > 0) {
    rt_usleep(refresh_us);
  }
  return timeout_count;
}

rt_uint rt_obtain_lock(rt_uint *lock)
{
  rt_uint got_cycle_lock = rt_await_zero_val(lock, 50000, 50);
  if (got_cycle_lock) {
    *lock = 1;
    return RU(1);
  }
  return RU(0);
}
void rt_release_lock(rt_params p, rt_uint *lock) { *lock = 0; }