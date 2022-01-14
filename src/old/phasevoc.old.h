#ifndef PHASEVOC_H
#define PHASEVOC_H

#include <fftw3.h>
#include <math.h>
#include <pthread.h>
// #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct STFTArgs {
  fftw_complex *out;
  size_t        frame_size;
  fftw_plan    *plan;
} stft_arg_t;

typedef struct STFTParams {
  size_t block_size, frame_size;
  float  overlap_factor, tone;
  size_t overlap_size, num_frames;
} stft_params_t;

stft_params_t stft_params(size_t block_size, size_t frame_size,
                          float overlap_factor);
fftw_plan    *create_plans(fftw_complex *out, const stft_params_t p);
fftw_plan    *create_plans_rev(fftw_complex *out, const stft_params_t p);
fftw_complex *stft_alloc_rfft_buffer(const stft_params_t p);
void          stft_copy_float_to_buffer(fftw_complex *dest, const float *buffer,
                                        stft_params_t p);
void stft_collapse_istft_to_floats(float *dest, const fftw_complex *out,
                                   stft_params_t p);
void stft_cleanup(fftw_complex *buffer, stft_params_t p, fftw_plan *plans,
                  fftw_plan *plans_rev);
void hanning(double *data, size_t len);
void hamming(double *data, size_t len);
void STFT_thread(void *arguments);
void STFT(fftw_complex *out, stft_params_t p, fftw_plan *plans);
void ISTFT(fftw_complex *out, stft_params_t p, fftw_plan *plans_rev);

#endif