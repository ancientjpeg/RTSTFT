/**
 * @file rt_utils.c
 * @author Jackson Kaplan (JwyattK@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-06-12
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "rtstft.h"
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

rt_real rt_ptocen(rt_real pitch_ratio) { return log2f(pitch_ratio) * 1200.f; }
rt_real rt_centop(rt_real cents) { return exp2f(cents / 1200.f); }
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