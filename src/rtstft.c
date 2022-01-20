#include "rtstft.h"

/*
  - General plan
    - user init
      - user creates an rt_params_t struct, passes it to be initialized
      - user is responsible for defining frame_size, block_size, overlap_factor
    -
  -
*/

rt_params rt_init(size_t block_size, int frame_size, float overlap_factor,
                  float sample_rate, int buffer_size)
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
  p->sample_rate    = sample_rate;
  p->overlap_size   = p->frame_size / p->overlap_factor;
  p->num_frames     = (p->block_size - p->frame_size) * p->overlap_size + 1;
  p->block          = rt_block_init(p->frame_size, p->num_frames);
  p->plan           = fftw_plan_r2r_1d(p->frame_size, p->block->frames[0],
                                       p->block->frames[0], FFTW_R2HC, FFTW_ESTIMATE);
  p->plan_inv       = fftw_plan_r2r_1d(p->frame_size, p->block->frames[0],
                                       p->block->frames[0], FFTW_HC2R, FFTW_ESTIMATE);
  p->in =
      rt_fifo_init(2 * (frame_size > buffer_size ? frame_size : buffer_size));
  p->out            = rt_fifo_init(p->in->size);
  p->pre_lerp_size  = p->scale_factor * p->block_size;
  p->pre_lerp_block = rt_fifo_init(p->pre_lerp_size);
  p->lerp_incr =
      (rt_real)p->pre_lerp_size / p->block_size; // incr = insize / outsize - 1
  p->first_frame = 1;
  return p;
}

rt_params rt_clean(rt_params p)
{
  p->block = rt_block_destroy(p->block);
  fftw_destroy_plan(p->plan);
  fftw_destroy_plan(p->plan_inv);
  free(p);
  return (rt_params)NULL;
}

int rt_insert_into_block(rt_params p, rt_real *data)
{
  int this_frame = p->block->next;
  memcpy(p->block->frames[this_frame], data, p->frame_size * sizeof(rt_real));
  fftw_execute_r2r(p->plan, p->block->frames[this_frame],
                   p->block->frames[this_frame]);
  // process, if last frame exists && firstFrame = 0
  fftw_execute_r2r(p->plan_inv, p->block->frames[this_frame],
                   p->block->frames[this_frame]);
  for (int i = 0; i < p->frame_size; i++) {
    p->block->frames[this_frame][i] /= p->frame_size;
  }
  p->block->next = (this_frame + 1) % p->num_frames;
  return p->block->next;
}

void rt_digest_frame(rt_params p)
{
  rt_insert_into_block(p, rt_fifo_get_head_ptr(p->in));
  rt_fifo_dequeue(p->in, p->overlap_size);
  // printf("\npayload: %zu blockfill: %zu\n", p->in->payload, p->in->payload);
}
void rt_assemble_frame(rt_params p)
{
  // ADD, by TIERING, onto the p->out FIFO
}

void rt_transform_test(rt_params p)
{
  for (int i = 0; i < p->num_frames; i++) {
    fftw_execute_r2r(p->plan, p->block->frames[i], p->block->frames[i]);
  }
}

void rt_cycle(rt_params p)
{

  while (p->in->payload >= p->frame_size) {
    // needs to be adjusted for STFT needing currentframe - 1
    rt_digest_frame(p);
    rt_assemble_frame(p);
  }
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

float get_fbin(int bin, rt_params p)
{
  return p->sample_rate / p->frame_size * bin;
}

void rt_lerp(rt_params p, rt_real *in, size_t in_size, rt_real *out,
             size_t out_size)
{
  rt_real interp_incr = (rt_real)in_size / (out_size - 1);
  rt_real j           = interp_incr;
  out[0]              = in[0];
  for (size_t i = 1; i < out_size - 1; i++) {
    size_t  curr = (size_t)j;
    rt_real mod  = j - floor(j);
    out[i]       = (in[curr + 1] - in[curr]) * mod + in[curr];
    j += p->lerp_incr;
  }
  out[out_size - 1] = in[in_size - 1];
}