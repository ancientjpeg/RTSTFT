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

#if INT_MAX < 2147483647
typedef unsigned long rt_uint;
#else
typedef unsigned int rt_uint;
#endif
#endif