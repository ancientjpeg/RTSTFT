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

void rt_fifo_enqueue(rt_fifo fifo, rt_real *data, int n)
{
  if (fifo->payload + n >= fifo->size) {
    n = fifo->size - fifo->payload;
    printf("fifo full.\n");
  }

  // this could be changed to a memcpy but i dont know if that's worth it
  for (int i = 0; i < n; i++) {
    fifo->tail              = ++fifo->tail % fifo->size;
    fifo->queue[fifo->tail] = data[i];
    ++fifo->payload;
    if (fifo->tail == fifo->head + 1 && fifo->payload != 1) {
      printf("unexpected error.\n");
      exit(1);
    }
  }
}
void rt_fifo_read(rt_fifo fifo, rt_real *dest, int n)
{
  for (int i = 0; i < n; i++) {
    int index = (fifo->head + i) % fifo->size;
    dest[i]   = fifo->queue[index];
  }
}
void rt_fifo_dequeue(rt_fifo fifo, int n)
{
  if (n >= fifo->payload) {
    fifo->head    = 0;
    fifo->tail    = 0;
    fifo->payload = 0;
    printf("fifo empty. exiting.\n");
    return;
  }
  fifo->head = (fifo->head + n) % fifo->size;
  fifo->payload -= n;
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

rt_real *rt_fifo_get_head_ptr(rt_fifo fifo) { return fifo->queue + fifo->head; }
rt_real *rt_fifo_get_tail_ptr(rt_fifo fifo) { return fifo->queue + fifo->head; }