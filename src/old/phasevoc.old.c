#include "phasevoc.h"
/*
  - first allocate array
  - then, create the plans
  - then you can execute the STFT
*/

// generate a params object
stft_params_t stft_params(size_t block_size, size_t frame_size,
                          float overlap_factor)
{
  stft_params_t p = {block_size, frame_size, overlap_factor};
  p.overlap_size  = p.frame_size / p.overlap_factor;
  p.num_frames    = (p.block_size - p.frame_size) / p.overlap_size + 1;
  return p;
}

fftw_plan *create_plans(fftw_complex *out, const stft_params_t p)
{
  fftw_plan *plans = (fftw_plan *)malloc(p.num_frames * sizeof(fftw_plan));
  for (size_t i = 0; i < p.num_frames; i++) {
    fftw_complex *this_ptr = out + (i * (p.frame_size / 2 + 1));
    plans[i] = fftw_plan_dft_r2c_1d(p.frame_size, (double *)this_ptr, this_ptr,
                                    FFTW_MEASURE);
  }
  return plans;
}

fftw_plan *create_plans_rev(fftw_complex *out, const stft_params_t p)
{
  fftw_plan *plans_rev = (fftw_plan *)malloc(p.num_frames * sizeof(fftw_plan));
  for (size_t i = 0; i < p.num_frames; i++) {

    fftw_complex *this_ptr = out + (i * (p.frame_size / 2 + 1));
    plans_rev[i]           = fftw_plan_dft_c2r_1d(p.frame_size, this_ptr,
                                                  (double *)this_ptr, FFTW_MEASURE);
  }
  return plans_rev;
}

// uses FFTW_alloc, just does the math for you
fftw_complex *stft_alloc_rfft_buffer(const stft_params_t p)
{
  fftw_complex *ptr = fftw_alloc_complex(p.num_frames * (p.frame_size / 2 + 1));
  return ptr;
}

void stft_copy_float_to_buffer(fftw_complex *dest, const float *buffer,
                               stft_params_t p)
{
  double *insert = (double *)dest;
  for (size_t i = 0; i < p.num_frames; i++) {
    size_t insert_index = i * (p.frame_size + 2);
    size_t buf_index    = i * p.overlap_size;
    for (size_t j = 0; j < p.frame_size + 2; j++) {
      if (j < p.frame_size) {
        insert[insert_index + j] = buffer[buf_index + j];
      }
      else {
        insert[insert_index + j] = 0.;
      }
    }
  }
}

// read the FFTW manual. it will all make sense one day
void stft_collapse_istft_to_floats(float *dest, const fftw_complex *out,
                                   stft_params_t p)
{
  const double *extract = (double *)out;
  for (size_t i = 0; i < p.block_size; i++) {
    dest[i] = 0.;
  }
  for (size_t i = 0; i < p.num_frames; i++) {
    size_t extract_index = i * (p.frame_size + 2);
    size_t dest_index    = i * p.overlap_size;
    for (size_t j = 0; j < p.frame_size; j++) {
      dest[dest_index + j] += extract[extract_index + j];
    }
  }
}

void stft_cleanup(fftw_complex *buffer, stft_params_t p, fftw_plan *plans,
                  fftw_plan *plans_rev)
{
  fftw_free(buffer);
  for (size_t i = 0; i < p.num_frames; i++) {
    if (plans)
      fftw_destroy_plan(plans[i]);
    if (plans_rev)
      fftw_destroy_plan(plans_rev[i]);
  }
  free(plans);
  free(plans_rev);
}

void hanning(double *data, size_t len)
{
  for (size_t n = 0; n < len; n++) {
    double hann = 0.5 - 0.5 * cos(2. * M_PI * n / (len - 1));
    data[n] *= hann;
  }
}

void hamming(double *data, size_t len)
{
  for (size_t n = 0; n < len; n++) {

    double hamm = 0.54 - 0.46 * cos(2. * M_PI * n / (len - 1));
    data[n] *= hamm;
  }
}

void STFT_thread(void *arguments)
{
  stft_arg_t *args = (stft_arg_t *)arguments;
  // WINDOWING HERE !!
  hanning((double *)args->out, args->frame_size);
  fftw_execute(*(args->plan));
}
void STFT(fftw_complex *out, stft_params_t p, fftw_plan *plans)
{
  for (size_t i = 0; i < p.num_frames; i++) {
    fftw_complex *this_ptr = out + i * (p.frame_size / 2 + 1);
    stft_arg_t    s        = (stft_arg_t){this_ptr, p.frame_size, plans + i};
    STFT_thread((void *)&s);
  }
}

void ISTFT_thread(void *arguments)
{
  stft_arg_t *args = (stft_arg_t *)arguments;
  // for (int i = 0; i < args->frame_size; i++) {
  //   if (i > 5) {
  //     double *val  = args->out[i];
  //     double  real = val[0];
  //     double  imag = val[1];
  //     // val[0]       = sqrt(real * real + imag * imag);
  //     // val[0] = 0;
  //     // val[1] = 0;
  //   }
  // }
  fftw_execute(*(args->plan));
  hanning((double *)args->out, args->frame_size);
}

void ISTFT(fftw_complex *out, stft_params_t p, fftw_plan *plans_rev)
{
  for (size_t i = 0; i < p.num_frames; i++) {

    fftw_complex *this_ptr = out + i * (p.frame_size / 2 + 1);
    stft_arg_t    s = (stft_arg_t){this_ptr, p.frame_size, plans_rev + i};
    ISTFT_thread((void *)&s);
  }
}