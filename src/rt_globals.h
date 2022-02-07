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

#include "pffft/pffft.h"
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

#if INT_MAX < 2147483647
typedef unsigned long rt_uint;
#else
typedef unsigned int rt_uint;
#endif
#endif