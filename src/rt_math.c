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
