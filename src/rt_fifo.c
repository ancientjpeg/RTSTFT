#include "rt_fifo.h"

rt_fifo rt_fifo_init(size_t size)
{
  rt_fifo fifo  = (rt_fifo)malloc(sizeof(rt_fifo_t));
  fifo->head    = 0;
  fifo->tail    = 0;
  fifo->payload = 0;
  fifo->size    = size;
  fifo->queue   = (rt_real *)fftw_alloc_real(size);
  return fifo;
}

void rt_fifo_enqueue(rt_fifo fifo, rt_real *data, int frame_size)
{
  for (int i = 0; i < frame_size; i++) {
    fifo->queue[fifo->tail] = data[i];
    fifo->tail              = (++fifo->tail) % fifo->size;
    if (fifo->tail == fifo->head) {
      printf("fifo empty. exiting.\n");
      break;
    }
    else if (++fifo->payload > fifo->size) {
      printf("fifo full. exiting.\n");
      break;
    }
  }
}
void rt_fifo_read(rt_fifo fifo, rt_real *dest, int frame_size)
{
  for (int i = 0; i < frame_size; i++) {
    int index = (fifo->head + i) % fifo->size;
    dest[i]   = fifo->queue[index];
  }
}
void rt_fifo_dequeue(rt_fifo fifo, int n)
{
  for (int i = 0; i < n; i++) {
    fifo->head = (++fifo->head) % fifo->size;
    if (fifo->tail == fifo->head) {
      printf("fifo empty. exiting.\n");
      break;
    }
  }
}
void rt_fifo_read_and_dequeue(rt_fifo fifo, rt_real *dest, int frame_size,
                              float overlap_factor)
{
  rt_fifo_read(fifo, dest, frame_size);
  rt_fifo_dequeue(fifo, (int)(frame_size / overlap_factor));
}

rt_fifo rt_fifo_destroy(rt_fifo fifo)
{
  fftw_free(fifo->queue);
  free(fifo);
  return (rt_fifo)NULL;
}