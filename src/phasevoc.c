#include "phasevoc.h"

void rtst_calculate_parameters(stft_params_t *params)
{
  params->overlap_size = params->frame_size / params->overlap_factor;
  params->num_frames =
      (params->block_size - params->frame_size) * params->overlap_size + 1;
}

// float *rtst_alloc_frame() {}