#include "phasevoc.h"

/*
  - General plan
    - user init
      - user creates an rt_params_t struct, passes it to be initialized
      - user is responsible for defining frame_size, block_size, overlap_factor
    -
  -
*/

rt_params_t rt_init(size_t block_size, unsigned int frame_size,
                    float overlap_factor)
{
  if (frame_size * sizeof(float) % 16 != 0 ||
      block_size * sizeof(float) % 16 != 0) {
    printf("Frames and blocks must be able to by byte-aligned to 16 bytes.");
    exit(1);
  }
  else if (!block_size || !frame_size || overlap_factor <= 0.f) {
    printf("Cannot have block size, frame size, or overlap factor be <= zero.");
    exit(1);
  }
  rt_params_t temp;
  temp.block_size     = block_size;
  temp.frame_size     = frame_size;
  temp.overlap_factor = overlap_factor;
  rt_calculate_parameters(&temp);
  return temp;
}

void rt_calculate_parameters(rt_params_t *params)
{
  params->overlap_size = params->frame_size / params->overlap_factor;
  params->num_frames =
      (params->block_size - params->frame_size) * params->overlap_size + 1;
}

void hanning(fftw_real *data, size_t len)
{
  for (size_t n = 0; n < len; n++) {
    fftw_real hann = 0.5 - 0.5 * cos(2. * M_PI * n / (len - 1));
    data[n] *= hann;
  }
}

void hamming(fftw_real *data, size_t len)
{
  for (size_t n = 0; n < len; n++) {
    fftw_real hamm = 0.54 - 0.46 * cos(2. * M_PI * n / (len - 1));
    data[n] *= hamm;
  }
}

// float *rtst_alloc_frame() {}