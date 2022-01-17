#ifndef RT_PHASEVOC_H
#define RT_PHASEVOC_H

#include "rt_block.h"
#include "rt_fifo.h"
#include "rt_globals.h"

typedef struct RTSTFT_Params {
  size_t    block_size;
  int       frame_size, overlap_size, num_frames;
  char      running;
  float     overlap_factor;
  fftw_plan plan, plan_inv;
  rt_block  block;
  rt_fifo   in, out;
} rt_params_t;
typedef rt_params_t *rt_params;

rt_params rt_init(size_t block_size, int frame_size, float overlap_factor);
void      rt_calculate_parameters(rt_params params);
void      hanning(rt_real *data, size_t len);
void      hamming(rt_real *data, size_t len);
rt_params rt_clean(rt_params p);
void      rt_lerp(rt_real *in, size_t in_size, rt_real *out, size_t out_size);

#endif