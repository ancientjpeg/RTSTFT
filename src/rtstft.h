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
  size_t block_size;
  int    frame_size, num_overlaps, hop_size, resynth_hop_size, num_frames,
      buffer_size;
  char      first_frame;
  rt_real   scale_factor, sample_rate, lerp_incr, lerp_pos;
  fftw_plan plan, plan_inv;
  rt_block  block;
  rt_fifo   in, out;
} rt_params_t;
typedef rt_params_t *rt_params;

rt_params rt_init(size_t block_size, int frame_size, int num_overlaps,
                  float sample_rate, int buffer_size);
void      rt_cycle(rt_params p);
rt_params rt_clean(rt_params p);

// ========  RT_BLOCK  ======== //
rt_block rt_block_init(rt_params p);
void     rt_block_insert(rt_params p);
rt_block rt_block_destroy(rt_block block);
int      rt_block_relative_frame(int num_frames, int frame, int offset);

// ======== MATH UTILS ======== //
void  rt_hanning(rt_real *data, size_t len);
void  rt_hamming(rt_real *data, size_t len);
float get_fbin(int bin, rt_params p);
void  rt_lerp(rt_params p, rt_real *out, size_t out_size, rt_real *in,
              size_t in_size);
#endif