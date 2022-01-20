#include "rt_fifo.h"

rt_fifo rt_fifo_init(size_t size)
{
  rt_fifo fifo = (rt_fifo)malloc(sizeof(rt_fifo_t));
  fifo->head   = 0;
  fifo->tail   = 0;
  fifo->empty  = 1;
  fifo->size   = size;
  fifo->queue  = (rt_real *)calloc(sizeof(rt_real), size);
  return fifo;
}
void rt_fifo_enqueue(rt_fifo fifo, rt_real *data, int n)
{
  rt_fifo_enqueue_staggered(fifo, data, n, n);
}

void rt_fifo_enqueue_staggered(rt_fifo fifo, rt_real *data, int n, int advance)
{
  if (!n) {
    return;
  }
  size_t payload = rt_fifo_get_payload(fifo);
  if (payload + advance >= fifo->size) {
    n = fifo->size - payload;
    printf("fifo full.\n");
  }
  fifo->empty      = 0;
  size_t tail_init = fifo->tail;
  for (int i = 0; i < n; i++) {
    fifo->queue[fifo->tail] += data[i];
    fifo->tail = ++fifo->tail % fifo->size;
    if (rt_fifo_get_payload(fifo) == 0) { // MARK FOR DELETION
      fprintf(stderr, "unexpected error.\n");
      exit(1);
    }
  }
  if (n != advance) {
    fifo->tail = (tail_init + advance) % fifo->size; // staggered tail
  }
}

void rt_fifo_read(rt_fifo fifo, rt_real *dest, int n)
{
  if (fifo->empty) {
    printf("Nothing to read.");
    return;
  }
  for (int i = 0; i < n; i++) {
    int index = (fifo->head + i) % fifo->size;
    dest[i]   = fifo->queue[index];
  }
}

void rt_fifo_dequeue(rt_fifo fifo, int n)
{
  if (fifo->empty) {
    printf("Nothing to dequeue.");
    return;
  }
  int target = (fifo->head + n) % fifo->size;
  if (n >= rt_fifo_get_payload(fifo)) {
    fifo->empty = 1;
    target      = fifo->tail;
    printf("fifo now empty. exiting.\n");
  }
  do {
    fifo->queue[fifo->head] = 0.;
    fifo->head              = (++fifo->head) % fifo->size;
  } while (fifo->head != target);
}

void rt_fifo_dequeue_staggered(rt_fifo fifo, rt_real *dest, int n, int advance)
{
  rt_fifo_read(fifo, dest, n);
  rt_fifo_dequeue(fifo, advance);
}

inline size_t rt_fifo_get_payload(rt_fifo fifo)
{
  size_t payload = 0;
  if (fifo->head == fifo->tail) {
    payload = fifo->empty ? 0 : fifo->size;
  }
  else if (fifo->head < fifo->tail) {
    payload = fifo->tail - fifo->head;
  }
  else if (fifo->head > fifo->tail) {
    payload = fifo->size - (fifo->head - fifo->tail);
  }
  else {
    fprintf(stderr, "critical error in rt_fifo_get_payload\n");
    exit(1);
  }
  return payload;
}

rt_fifo rt_fifo_destroy(rt_fifo fifo)
{
  free(fifo->queue);
  free(fifo);
  return (rt_fifo)NULL;
}

rt_real *rt_fifo_get_head_ptr(rt_fifo fifo) { return fifo->queue + fifo->head; }
rt_real *rt_fifo_get_tail_ptr(rt_fifo fifo) { return fifo->queue + fifo->head; }