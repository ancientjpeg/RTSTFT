#include "rtstft.h"

/*
  - General plan
    - user init
      - user creates an rt_params_t struct, passes it to be initialized
      - user is responsible for defining frame_size, block_size, num_overlaps
    -
  -
*/

rt_params rt_init(size_t block_size, int frame_size, int num_overlaps,
                  float sample_rate, int buffer_size)
{
  if (frame_size * sizeof(float) % 16 != 0 ||
      block_size * sizeof(float) % 16 != 0) {
    fprintf(stderr,
            "Frames and blocks must be able to by byte-aligned to 16 bytes.");
    exit(1);
  }
  else if (!block_size || !frame_size || num_overlaps <= 0.f) {
    fprintf(
        stderr,
        "Cannot have block size, frame size, or overlap factor be <= zero.");
    exit(1);
  }
  rt_params p         = malloc(sizeof(rt_params_t));
  p->scale_factor     = 1.0;
  p->block_pos        = 0;
  p->block_size       = block_size;
  p->frame_size       = frame_size;
  p->num_overlaps     = num_overlaps;
  p->sample_rate      = sample_rate;
  p->hop_size         = p->frame_size / p->num_overlaps;
  p->resynth_hop_size = lround(p->hop_size * p->scale_factor);
  p->num_frames       = (p->block_size - p->frame_size) / p->hop_size + 1;
  p->block            = rt_block_init(p);
  p->plan             = fftw_plan_r2r_1d(p->frame_size, p->block->frames[0],
                                         p->block->frames[0], FFTW_R2HC, FFTW_ESTIMATE);
  p->plan_inv         = fftw_plan_r2r_1d(p->frame_size, p->block->frames[0],
                                         p->block->frames[0], FFTW_HC2R, FFTW_ESTIMATE);
  p->in =
      rt_fifo_init(2 * (frame_size > buffer_size ? frame_size : buffer_size));
  p->out           = rt_fifo_init(p->in->size);
  p->pre_lerp_size = p->frame_size + p->resynth_hop_size * (p->num_frames - 1);
  p->lerp_incr     = (rt_real)(p->pre_lerp_size - 1) / (p->block_size - 1);
  p->lerp_pos      = 0.;
  p->lerp_samples_read = 0.;
  p->first_frame       = 1;
  return p;
}

rt_params rt_clean(rt_params p)
{
  p->block = rt_block_destroy(p->block);
  p->in    = rt_fifo_destroy(p->in);
  p->out   = rt_fifo_destroy(p->out);
  fftw_destroy_plan(p->plan);
  fftw_destroy_plan(p->plan_inv);
  free(p);
  return (rt_params)NULL;
}

void rt_digest_frame(rt_params p)
{
  int this_frame = p->block->next_write;
  int last_frame = rt_block_relative_frame(p->num_frames, this_frame, -1);
  rt_fifo_dequeue_staggered(p->in, p->block->frames[this_frame], p->frame_size,
                            p->hop_size);
  ++p->block->next_write;
  p->block->frame_data[this_frame] |= RT_FRAME_IS_FILLED;
  rt_hanning(p->block->frames[this_frame], p->frame_size);
  fftw_execute_r2r(p->plan, p->block->frames[this_frame],
                   p->block->frames[this_frame]);
  p->block->frame_data[this_frame] |= RT_FRAME_IS_TRANSFORMED;
  p->block->next_write = rt_block_relative_frame(p->num_frames, this_frame, 1);
}
void rt_process_frame(int frame) { return; }
void rt_process_frames(rt_params p)
{
  int this_frame = p->block->next_unprocessed;
  if (p->first_frame &&
      p->block->frame_data[this_frame] & RT_FRAME_IS_TRANSFORMED) {
    rt_process_frame(this_frame);
    p->block->frame_data[this_frame] |= RT_FRAME_IS_PROCESSED;
    p->first_frame = 0;
    this_frame     = rt_block_relative_frame(p->num_frames, this_frame, 1);
    p->block->next_unprocessed = this_frame;
  }
  int last_frame = rt_block_relative_frame(p->num_frames, this_frame, -1);

  while (p->block->frame_data[last_frame] & RT_FRAME_IS_PROCESSED &&
         p->block->frame_data[this_frame] & RT_FRAME_IS_TRANSFORMED) {
    rt_process_frame(this_frame);
    p->block->frame_data[this_frame] |= RT_FRAME_IS_PROCESSED;

    fftw_execute_r2r(p->plan_inv, p->block->frames[last_frame],
                     p->block->frames[last_frame]);
    rt_hanning(p->block->frames[last_frame], p->frame_size);
    for (int i = 0; i < p->frame_size; i++) {
      p->block->frames[last_frame][i] /= p->frame_size; // normalization
    }
    p->block->frame_data[last_frame] |= RT_FRAME_IS_INVERTED;

    last_frame = this_frame;
    this_frame = rt_block_relative_frame(p->num_frames, this_frame, 1);
  }
  p->block->next_unprocessed = this_frame;
}

void rt_assemble_frame(rt_params p)
{
  int this_frame = p->block->next_unread;
  while (p->block->frame_data[this_frame] & RT_FRAME_IS_INVERTED) {
    rt_fifo_enqueue_staggered(p->out, p->block->frames[this_frame],
                              p->frame_size, p->hop_size);
    p->block->frame_data[this_frame] = 0;
    this_frame = rt_block_relative_frame(p->num_frames, this_frame, 1);
  }
  p->block->next_unread = this_frame;
}

void pr(FILE *stream, rt_real *arr, size_t len)
{
  fprintf(stream, "[ \n");
  for (size_t i = 0; i < len; i++) {
    if (i % 16 == 15) {
      fprintf(stream, "    ");
    }
    fprintf(stream, "%.1f, ", arr[i]);
    if (i % 16 == 15 && i != len - 1) {
      fprintf(stream, "\n");
    }
  }
  fprintf(stream, "\n],\n");
}
void rt_cycle(rt_params p)
{
  // TODO assemble frame through out fifo back to wav
  // latency ??
  // manage control rates between the buffers
  while (rt_fifo_get_payload(p->in) >= p->frame_size) {
    // needs to be adjusted for STFT needing currentframe - 1
    rt_digest_frame(p);
    rt_process_frames(p);
    // p->out needs to be zeroed before insertion
    // i guess all the FIFOs will need that
    rt_assemble_frame(p);
  }
}
