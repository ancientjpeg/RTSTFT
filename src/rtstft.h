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
  rt_uint frame_size, overlap_factor, hop_a, hop_s, buffer_size, hop_pos,
      latency_block;
  char      first_frame;
  rt_real   scale_factor, scale_factor_actual, sample_rate, mod_track;
  fftw_plan plan, plan_inv;
  rt_block  block;
  rt_fifo   in, out;
} rt_params_t;
typedef rt_params_t *rt_params;

rt_params            rt_init(rt_uint frame_size, rt_uint overlap_factor,
                             rt_uint buffer_size, float sample_rate);
void                 rt_cycle(rt_params p, rt_real *buffer, rt_uint buffer_len);
rt_params            rt_clean(rt_params p);

/* ========  RT_BLOCK  ======== */
rt_block rt_block_init(rt_params p, rt_uint num_frames);
void     rt_block_insert(rt_params p);
rt_block rt_block_destroy(rt_block block);
rt_uint  rt_block_relative_frame(rt_uint num_frames, rt_uint frame, int offset);
void     rt_block_convert_frame(rt_params p, rt_uint frame);
void     rt_block_process_frame(rt_params p, rt_uint frame);
void     rt_block_revert_frame(rt_params p, rt_uint frame);

/* ======== MATH UTILS ======== */
void  rt_hanning(rt_real *data, rt_uint len);
void  rt_hamming(rt_real *data, rt_uint len);
float get_fbin(int bin, rt_params p);
void  rt_lerp(rt_params p, rt_real *out, rt_uint out_size, rt_real *in,
              rt_uint in_size);
#endif