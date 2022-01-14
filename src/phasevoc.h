#include <fftw3.h>
#include <stdlib.h>
#include <string.h>

typedef struct STFTParams {
  size_t block_size;
  int    frame_size;
  float  overlap_factor;
  int    overlap_size, num_frames;
} stft_params_t;

void rtst_calculate_parameters(stft_params_t *params);
