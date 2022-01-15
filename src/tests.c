#include "phasevoc.h"
#include "wavfile.h"
#include <time.h>

int main()
{
  // rt_params_t p   = rt_init(1U << 20, 2048, 4.f);
  // WAV         wav = read_from_wav("in.wav", &p);
  // write_to_wav("out.wav", &wav);
  clock_t t;
  int     size = 128;
  double *n    = fftw_alloc_real(size);
  double *n2   = fftw_alloc_real(size);
  for (int i = 0; i < size; i++) {
    n[i]  = sin(i);
    n2[i] = n[i];
  }
  printf("%f %f %f %f \n", n[0], n2[0], n[1], n2[1]);
  t            = clock();
  fftw_plan p  = fftw_plan_r2r_1d(size, n, n, FFTW_R2HC, FFTW_ESTIMATE);
  fftw_plan pr = fftw_plan_r2r_1d(size, n, n, FFTW_HC2R, FFTW_ESTIMATE);
  // fftw_execute(p);
  // fftw_execute(pr);
  fftw_execute_r2r(p, n, n);
  fftw_execute_r2r(p, n2, n2);
  fftw_execute_r2r(pr, n, n);
  fftw_execute_r2r(pr, n2, n2);
  printf("seconds: %.10f\n", (double)(clock() - t) / CLOCKS_PER_SEC);
  printf("%f %f %f %f \n", n[0] / size, n2[0] / size, n[1] / size,
         n2[1] / size);
  return 0;
}
