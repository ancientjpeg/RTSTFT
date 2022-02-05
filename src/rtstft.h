#ifndef RT_PHASEVOC_H
#define RT_PHASEVOC_H

#include "rt_fifo.h"
#include "rt_framebuf.h"
#include "rt_globals.h"

/*
  - scale_factor is how much the STFT is scaling
    - i.e. 2. for and octave up shift, 1.0595 for 1 st etc.
*/

typedef struct RTSTFT_Channel {
  rt_framebuf framebuf;
  fftw_plan   plan, plan_inv;
  rt_fifo     in, pre_lerp, out;
  char        first_frame;
} rt_chan_t;
typedef rt_chan_t *rt_chan;

typedef struct RTSTFT_Params {
  rt_uint num_chans, fft_size, frame_size, frame_max, overlap_factor,
      pad_factor, pad_offset, hop_a, hop_s, buffer_size;
  rt_real  scale_factor, sample_rate;
  rt_chan *chans;
} rt_params_t;
typedef rt_params_t *rt_params;

rt_params rt_init(rt_uint num_channels, rt_uint frame_size, rt_uint buffer_size,
                  rt_uint overlap_factor, rt_uint pad_factor,
                  float sample_rate);
void      rt_cycle(rt_params p, rt_real **buffers, rt_uint num_buffers,
                   rt_uint buffer_len);
void      rt_cycle_single(rt_params p, rt_real *buffer, rt_uint buffer_len);
void      rt_cycle_offset(rt_params p, rt_real **buffers, rt_uint num_buffers,
                          rt_uint buffer_len, rt_uint sample_offset);
void      rt_cycle_chan(rt_params p, rt_uint channel_index, rt_real *buffer,
                        rt_uint buffer_len);
rt_params rt_clean(rt_params p);

/* ========  rt_framebuf  ======== */
rt_framebuf rt_framebuf_init(rt_params p, rt_uint num_frames);
rt_framebuf rt_framebuf_destroy(rt_framebuf framebuf);
rt_uint     rt_framebuf_relative_frame(rt_framebuf framebuf, rt_uint frame,
                                       int offset);
void        rt_framebuf_convert_frame(rt_params p, rt_chan c, rt_uint frame);
void        rt_framebuf_process_frame(rt_params p, rt_chan c, rt_uint frame);
void        rt_framebuf_revert_frame(rt_params p, rt_chan c, rt_uint frame);

/* ======== MATH UTILS ======== */
void rt_hanning(rt_real *data, rt_uint len);
void rt_hamming(rt_real *data, rt_uint len);
#define rt_window rt_hanning

/* ======== MISC UTILS ======== */
rt_uint rt_real_size();
#endif