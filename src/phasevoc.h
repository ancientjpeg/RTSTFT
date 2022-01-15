#ifndef RT_PHASEVOC_H
#define RT_PHASEVOC_H

#include "rt_fifo.h"
#include "rt_globals.h"

typedef struct RTSTFT_Params {
  size_t       block_size;
  unsigned int frame_size, overlap_size, num_frames;
  float        overlap_factor;
  fftw_plan    plan_forward, plan_reverse;

} rt_params_t;

rt_params_t rt_init(size_t block_size, unsigned int frame_size,
                    float overlap_factor);
void        rt_calculate_parameters(rt_params_t *params);
void        hanning(fftw_real *data, size_t len);
void        hamming(fftw_real *data, size_t len);

#endif