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
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef RT_DOUBLE
typedef double rt_real;
#else
typedef float        rt_real;
#endif

#if UINT_MAX < 4294967295
typedef unsigned long rt_uint;
#define RU(literal) (literal##UL)
#define ru "%lu"
#else
typedef unsigned int rt_uint;
#define RU(literal) (literal##U)
#define ru "%u"
#endif

#define RT_UINT_FALSE ((rt_uint)-1)
#define RT_REAL_ERR ((rt_real)(0. / 0.))
#define RT_INT_ERR INT_MIN

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

typedef enum RT_PARAM_FLAVOR {
  RT_SCALE_FACTOR_MOD,
  RT_RETENTION_MOD,
  RT_PHASE_MOD,
  RT_PHASE_CHAOS,
  RT_PARAM_GAIN_MOD,
  RT_PARAM_GATE_MOD,
  RT_PARAM_LIMIT_MOD,
  RT_PARAM_FLAVOR_COUNT,
  RT_PARAM_FLAVOR_UNDEFINED
} rt_param_flavor_t;

typedef enum RT_MANIP_FLAVORS {
  RT_MANIP_GAIN,
  RT_MANIP_GATE,
  RT_MANIP_LIMIT,
  RT_MANIP_FLAVOR_COUNT,
  RT_MANIP_FLAVOR_UNDEFINED
} rt_manip_flavor_t;

typedef struct RTSTFT_Params_Listener_Return_Data {
  rt_param_flavor_t param_flavor;
  rt_manip_flavor_t manip_flavor;
  rt_real           param_value;
} rt_listener_return_t;

typedef struct RTSTFT_Params_Listener {
  void *listener_obj;
  void (*listener_callback)(void *, rt_listener_return_t);
} rt_listener_t;

#endif
