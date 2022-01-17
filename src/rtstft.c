#include "rtstft.h"

/*
  - General plan
    - user init
      - user creates an rt_params_t struct, passes it to be initialized
      - user is responsible for defining frame_size, block_size, overlap_factor
    -
  -
*/

rt_params rt_init(size_t block_size, int frame_size, float overlap_factor)
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
  rt_params p       = malloc(sizeof(rt_params_t));
  p->block_size     = block_size;
  p->frame_size     = frame_size;
  p->overlap_factor = overlap_factor;
  rt_calculate_parameters(p);
  p->block    = rt_block_init(p->frame_size, p->num_frames);
  p->plan     = fftw_plan_r2r_1d(p->frame_size, p->block->frames[0],
                                 p->block->frames[0], FFTW_R2HC, FFTW_ESTIMATE);
  p->plan_inv = fftw_plan_r2r_1d(p->frame_size, p->block->frames[0],
                                 p->block->frames[0], FFTW_HC2R, FFTW_ESTIMATE);
  return p;
}
void rt_calculate_parameters(rt_params p)
{
  p->overlap_size = p->frame_size / p->overlap_factor;
  p->num_frames   = (p->block_size - p->frame_size) * p->overlap_size + 1;
}

rt_params rt_clean(rt_params p)
{
  p->block = rt_block_destroy(p->block);
  fftw_destroy_plan(p->plan);
  fftw_destroy_plan(p->plan_inv);
  free(p);
  return (rt_params)NULL;
}

int rt_insert_into_block(rt_params p, rt_real *data, int size)
{
  int this_frame = p->block->first_available;
  memcpy(p->block->frames[this_frame], data, size * sizeof(rt_real));
  p->block->first_available = (this_frame + 1) % p->num_frames;
  return p->block->first_available;
}

void hanning(rt_real *data, size_t len)
{
  for (size_t n = 0; n < len; n++) {
    rt_real hann = 0.5 - 0.5 * cos(2. * M_PI * n / (len - 1));
    data[n] *= hann;
  }
}

void hamming(rt_real *data, size_t len)
{
  for (size_t n = 0; n < len; n++) {
    rt_real hamm = 0.54 - 0.46 * cos(2. * M_PI * n / (len - 1));
    data[n] *= hamm;
  }
}

void rt_lerp(rt_real *in, size_t in_size, rt_real *out, size_t out_size)
{
  rt_real interp_incr = (rt_real)in_size / (out_size - 1);
  rt_real j           = interp_incr;
  out[0]              = in[0];
  for (size_t i = 1; i < out_size - 1; i++) {
    size_t  curr = (size_t)j;
    rt_real mod  = j - floor(j);
    out[i]       = (in[curr + 1] - in[curr]) * mod + in[curr];
    j += interp_incr;
  }
  out[out_size - 1] = in[in_size - 1];
}