#ifndef RT_PHASEVOC_H
#define RT_PHASEVOC_H

#include "rt_fifo.h"
#include "rt_framebuf.h"
#include "rt_globals.h"

/*
  - scale_factor is how much the STFT is scaling
    - i.e. 2. for and octave up shift, 1.0595 for 1 st etc.
*/
typedef struct RTSTFT_Params {
  rt_uint frame_size, overlap_factor, hop_a, hop_s, buffer_size, latency_block;
  char    first_frame;
  rt_real scale_factor, sample_rate, mod_track;
  rt_framebuf framebuf;
  fftw_plan   plan, plan_inv;
  rt_fifo     in, pre_lerp, out;
} rt_params_t;
typedef rt_params_t *rt_params;

rt_params            rt_init(rt_uint frame_size, rt_uint overlap_factor,
                             rt_uint buffer_size, float sample_rate, float scale_factor);
void                 rt_cycle(rt_params p, rt_real *buffer, rt_uint buffer_len);
rt_params            rt_clean(rt_params p);

/* ========  rt_framebuf  ======== */
rt_framebuf rt_framebuf_init(rt_params p, rt_uint num_frames);
void        rt_framebuf_insert(rt_params p);
rt_framebuf rt_framebuf_destroy(rt_framebuf framebuf);
rt_uint     rt_framebuf_relative_frame(rt_framebuf framebuf, rt_uint frame,
                                       int offset);
void        rt_framebuf_convert_frame(rt_params p, rt_uint frame);
void        rt_framebuf_process_frame(rt_params p, rt_uint frame);
void        rt_framebuf_revert_frame(rt_params p, rt_uint frame);

/* ======== MATH UTILS ======== */
void  rt_hanning(rt_real *data, rt_uint len);
void  rt_hamming(rt_real *data, rt_uint len);
float get_fbin(int bin, rt_params p);
void  rt_lerp(rt_params p, rt_real *out, rt_uint out_size, rt_real *in,
              rt_uint in_size);
#endif