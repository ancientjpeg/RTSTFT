#include "rtstft.h"

/*
  - General plan
    - user init
      - user creates an rt_params_t struct, passes it to be initialized
      - user is responsible for defining frame_size, block_size, overlap_factor,
        - (eventually) pad_factor
  - current issues
    - STFT aliasing
      - even at 1024 fft-size, severe aliasing w/ sine waves
        - 1024 fft gives 512 real bands
        - 1kHz -> 1.26 kHz (major third)
          - 1kHz aliased between 990 and 1033 Hz
          - 1.26kHz aliased between 1248 and 1290
    - pitch issues
      - even without clear aliasing, serious pitch drift sometimes
*/

void rt_digest_frame(rt_params p, rt_chan c)
{
  rt_uint this_frame = c->framebuf->next_write;
  if (p->pad_factor != 0) {
    rt_uint i;
    for (i = 0; i < p->pad_offset; i++) {
      c->framebuf->frames[this_frame][i]                   = 0.;
      c->framebuf->frames[this_frame][p->fft_size - 1 - i] = 0.;
    }
    for (i = 0; i < p->fft_size; i++) {
      c->framebuf->frames[this_frame][i] = 0.;
    }
  }
  rt_fifo_dequeue_staggered(c->in,
                            c->framebuf->frames[this_frame] + p->pad_offset,
                            p->frame_size, p->hop_a);
  c->framebuf->frame_data[this_frame] |= RT_FRAME_IS_FILLED;
  c->framebuf->next_write =
      rt_framebuf_relative_frame(c->framebuf, this_frame, 1);
  rt_framebuf_convert_frame(p, c, this_frame);
}
void rt_process_frame(rt_params p, rt_chan c)
{
  rt_uint this_frame = c->framebuf->next_unprocessed;
  rt_framebuf_process_frame(p, c, this_frame);
}

void rt_assemble_frame(rt_params p, rt_chan c)
{
  rt_uint this_frame = c->framebuf->next_unread;
  rt_framebuf_revert_frame(p, c, this_frame);
  if (c->framebuf->frame_data[this_frame] & RT_FRAME_IS_INVERTED) {
    rt_fifo_enqueue_staggered(c->pre_lerp,
                              c->framebuf->frames[this_frame] + p->pad_offset,
                              p->frame_size, p->hop_s);
    c->framebuf->next_unread =
        rt_framebuf_relative_frame(c->framebuf, this_frame, 1);
  }
  else {
    fprintf(stderr, "Frame error in assemble.\n");
  }
}

void rt_lerp_read_out(rt_params p, rt_chan c, rt_uint num_hops)
{
  rt_uint          input_size  = p->hop_s * num_hops;
  rt_uint          output_size = p->hop_a * num_hops;
  register rt_real local_incr  = (rt_real)(input_size - 1) / (output_size - 1);
  register rt_uint i;
  for (i = 0; i < output_size - 1; i++) {
    rt_real pos       = i * local_incr;
    rt_uint pos_floor = (rt_uint)pos;
    rt_real mod       = pos - pos_floor;
    rt_uint i_input =
        rt_fifo_new_pos(c->pre_lerp, c->pre_lerp->head, pos_floor);
    rt_uint i_input_1 =
        rt_fifo_new_pos(c->pre_lerp, c->pre_lerp->head, pos_floor + 1);
    rt_real result =
        (c->pre_lerp->queue[i_input_1] - c->pre_lerp->queue[i_input]) * mod +
        c->pre_lerp->queue[i_input];

    rt_fifo_enqueue_one(c->out, &result);
  }
  rt_fifo_enqueue_one(c->out,
                      c->pre_lerp->queue + rt_fifo_new_pos(c->pre_lerp,
                                                           c->pre_lerp->head,
                                                           input_size - 1));

  rt_fifo_dequeue(c->pre_lerp, input_size);
}

void rt_cycle_chan(rt_params p, rt_uint channel_index, rt_real *buffer,
                   rt_uint buffer_len)
{
  rt_chan  c               = channel_index;
  rt_real *buffer_orig     = buffer;
  rt_uint  buffer_len_save = buffer_len;
  while (buffer_len > 0) {
    rt_fifo_enqueue_one(c->in, buffer);
    char check = 0;

    while (rt_fifo_payload(c->in) >= p->frame_size) {
      rt_digest_frame(p, c);
      char was_first = c->first_frame;
      rt_process_frame(p, c);
      if (!was_first) {
        rt_assemble_frame(p, c);
      }
      if (rt_fifo_readable(c->pre_lerp) >= p->hop_s * p->overlap_factor) {
        rt_lerp_read_out(p, c, p->overlap_factor);
      }
      if (++check > 1) {
        fprintf(stderr, "Extracted more than one frame.\n");
        exit(1);
      }
    }
    if (rt_fifo_readable(c->out)) {
      rt_fifo_dequeue_one(c->out, buffer);
    }
    else {
      *buffer = 0.;
    }
    ++buffer;
    --buffer_len;
  }
}

void rt_cycle(rt_params p, rt_real **buffers, rt_uint num_buffers,
              rt_uint buffer_len)
{
  rt_uint i;
  for (i = 0; i < num_buffers; i++) {
    rt_cycle_chan(p, p->chans[i], buffers[i], buffer_len);
  }
}
