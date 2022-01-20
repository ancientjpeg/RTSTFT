#ifndef RT_GLOBALS_H
#define RT_GLOBALS_H

#include <fftw3.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef RT_USING_DOUBLE
typedef double rt_real;
#else
typedef float rt_real;
#define fftw_plan_r2r_1d fftwf_plan_r2r_1d
#define fftw_plan fftwf_plan
#define fftw_execute_r2r fftwf_execute_r2r
#define fftw_alloc_real fftwf_alloc_real
#define fftw_free fftwf_free
#define fftw_destroy_plan fftwf_destroy_plan
#endif
/*  - to convert to float, simply:
      - change rt_real to float
      - change all fftw_* to fftwf_*
      - at least I hope that's how it works
*/

#endif