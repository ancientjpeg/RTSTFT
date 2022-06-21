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
/* library includes */
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* type defines */
#ifdef RT_DOUBLE
#define RR(literal) (literal)
typedef double rt_real;
#else
#define RR(literal) (literal##f)
typedef float        rt_real;
#endif
#define rr "%f"

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
#define RT_REAL_ERR NAN
#define RT_INT_ERR INT_MIN

/* stft-related defines */
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

/**
 * dB scaling defines
 * not currently used in-lib, but kept here in case of a need for integration
 * between the lib and rtstft_ctl
 */
#define RT_DB_MIN (RR(-100.))
#define RT_DB_MAX (RR(12.))
/* amplitude representation of RT_DB_MIN (-100.f) */
#define RT_DB_MIN_AMP (RR(1e-05))

/* define RT_ALLOW_NINF_DB if you'd like to allow manips to be == 0. */
#ifndef RT_ALLOW_NINF_DB
#define RT_ENFORCE_DB_MIN
#endif

typedef enum RT_PARAM_FLAVOR {
  RT_SCALE_FACTOR_MOD,
  RT_RETENTION_MOD,
  RT_PHASE_MOD,
  RT_PHASE_CHAOS,
  RT_PARAM_GAIN_MOD,
  RT_PARAM_GATE_MOD,
  RT_PARAM_LIMIT_MOD,
  RT_DRY_WET,
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

/* rt_parser defines */
#define RT_CMD_ARGC_MAX 10
#define RT_CMD_ARG_LEN_MAX 10
#define RT_CMD_BUFFER_LEN (RT_CMD_ARGC_MAX * RT_CMD_ARG_LEN_MAX)
#define RT_CMD_NAME_LEN 10
#define RT_CMD_MAX_OPTS 4
#define RT_CMD_STROPC_MAX 2
#define RT_CMD_OPT_ARGC_MAX 3
#define RT_CMD_COMMAND_ARGC_MAX 3

#define RT_CMD_ALL_COMMANDS_COUNT (RU(7))
#define RT_CMD_MAX_SEARCH_DEPTH (rt_log2_floor(RT_CMD_ALL_COMMANDS_COUNT))

/* for integrating an external parameter manager with rt_cmd */
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
