/**
 * @file rt_globals.h
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RT_GLOBALS_H
#define RT_GLOBALS_H

#include <fftw3.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef RT_DOUBLE
typedef double rt_real;
#else
typedef float        rt_real;
#define fftw_plan_r2r_1d fftwf_plan_r2r_1d
#define fftw_plan fftwf_plan
#define fftw_execute_r2r fftwf_execute_r2r
#define fftw_alloc_real fftwf_alloc_real
#define fftw_free fftwf_free
#define fftw_destroy_plan fftwf_destroy_plan
#define fftw_print_plan fftwf_print_plan
#endif
/*  - to convert to float, simply:
      - change rt_real to float
      - change all fftw_* to fftwf_*
      - at least I hope that's how it works
*/
#if INT_MAX < 2147483647
typedef unsigned long rt_uint;
#else
typedef unsigned int rt_uint;
#endif
#endif