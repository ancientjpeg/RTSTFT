#ifndef RT_PHASEVOC_H
#define RT_PHASEVOC_H

#include "rt_block.h"
#include "rt_fifo.h"
#include "rt_globals.h"

/*
  - scale_factor is how much the STFT is scaling
    - i.e. 2. for and octave up shift, 1.0595 for 1 st etc.
*/
typedef struct RTSTFT_Params {
  size_t    block_size, pre_lerp_size;
  int       frame_size, overlap_size, num_frames, buffer_size;
  char      first_frame;
  rt_real   overlap_factor, scale_factor, sample_rate, lerp_incr;
  fftw_plan plan, plan_inv;
  rt_block  block;
  rt_fifo   in, out, pre_lerp_block;
} rt_params_t;
typedef rt_params_t *rt_params;

rt_params rt_init(size_t block_size, int frame_size, float overlap_factor,
                  float sample_rate, int buffer_size);
void      rt_cycle(rt_params p);
void      hanning(rt_real *data, size_t len);
void      hamming(rt_real *data, size_t len);
rt_params rt_clean(rt_params p);
void      rt_lerp(rt_params p, rt_real *in, size_t in_size, rt_real *out,
                  size_t out_size);
#endif