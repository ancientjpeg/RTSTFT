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

#ifdef RT_DOUBLE
typedef double rt_real;
#else
typedef float        rt_real;
#endif

#if UINT_MAX < 4294967295
typedef unsigned long rt_uint;
#else
typedef unsigned int rt_uint;
#endif

#define RT_UINT_FALSE ((rt_uint)-1)

#ifdef RT_FFT_MIN_POW_OVERRIDE
#define RT_FFT_MIN_POW RT_FFT_MIN_POW_OVERRIDE
#else
#define RT_FFT_MIN_POW 5
#endif

#ifdef RT_FFT_MAX_POW_OVERRIDE
#define RT_FFT_MAX_POW RT_FFT_MAX_POW_OVERRIDE
#else
#define RT_FFT_MAX_POW 16
#endif

#ifdef RT_OVERLAP_MIN_OVERRIDE
#define RT_OVERLAP_MIN RT_OVERLAP_MIN_OVERRIDE
#else
#define RT_OVERLAP_MIN 2
#endif

#ifdef RT_OVERLAP_MAX_OVERRIDE
#define RT_OVERLAP_MAX RT_OVERLAP_MAX_OVERRIDE
#else
#define RT_OVERLAP_MAX 16
#endif

#ifdef RT_PAD_MAX_OVERRIDE
#define RT_PAD_MAX RT_PAD_MAX_OVERRIDE
#else
#define RT_PAD_MAX 3
#endif

#endif