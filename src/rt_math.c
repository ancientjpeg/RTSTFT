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

rt_uint rt_log2_floor(rt_uint num)
{
  int power = 0;
  while (num >> (power) > 0) power++;
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
