#include "rtstft.h"

#define rt_fifo_head_ptr(f) ((f)->queue + (f)->head)
#define rt_fifo_write_ptr(f) ((f)->queue + (f)->head)
#define rt_fifo_tail_ptr(f) ((f)->queue + (f)->tail)
#define rt_fifo_new_pos(f, i, n) (((i) + (n)) % (f)->size)
#define rt_fifo_get_diff(f, start, end)                                        \
  ((start) <= (end) ? (end) - (start) : (f)->size - ((start) - (end)))

rt_fifo rt_fifo_init(size_t size)
{
  rt_fifo fifo     = (rt_fifo)malloc(sizeof(rt_fifo_t));
  fifo->head       = 0;
  fifo->write_pos  = 0;
  fifo->tail       = 0;
  fifo->empty      = 1;
  fifo->read_empty = 1;
  fifo->size       = size;
  fifo->queue      = (rt_real *)calloc(sizeof(rt_real), size);
  return fifo;
}
void rt_fifo_enqueue(rt_fifo fifo, rt_real *data, int n)
{
  rt_fifo_enqueue_staggered(fifo, data, n, n);
}

void rt_fifo_enqueue_staggered(rt_fifo fifo, rt_real *data, int n, int advance)
{

  size_t payload = rt_fifo_payload(fifo);
  if (payload + advance >= fifo->size) {
    n       = fifo->size - payload;
    advance = advance > n ? n : advance;

    fprintf(stderr, "fifo full. %i\n", n);
  }
  fifo->empty        = 0;
  fifo->read_empty   = 0;
  size_t write_init  = fifo->write_pos;
  char   passed_tail = 0;
  for (int i = 0; i < n; i++) {
    fifo->queue[fifo->write_pos] += data[i];
    if (fifo->write_pos == fifo->tail) {
      passed_tail = 1;
    }
    fifo->write_pos = rt_fifo_new_pos(fifo, fifo->write_pos, 1);
    if (rt_fifo_payload(fifo) == 0) { // MARK FOR DELETION
      fprintf(stderr, "unexpected error.\n");
      exit(1);
    }
  }
  fifo->tail = passed_tail ? fifo->write_pos : fifo->tail;
  if (n != advance) {
    fifo->write_pos = rt_fifo_new_pos(fifo, write_init, advance);
  }
}

void rt_fifo_read(rt_fifo fifo, rt_real *dest, int n)
{
  if (fifo->read_empty) {
    printf("Nothing to read.");
    return;
  }
  else if (n > rt_fifo_readable_payload(fifo)) {
    fprintf(stderr, "Error: cannot read beyond the current write pointer.");
    exit(1);
  }
  for (int i = 0; i < n; i++) {
    int index = rt_fifo_new_pos(fifo, fifo->head, i);
    dest[i]   = fifo->queue[index];
  }
}

void rt_fifo_dequeue(rt_fifo fifo, int n)
{
  if (fifo->empty) {
    printf("Nothing to dequeue.");
    return;
  }
  int target = rt_fifo_new_pos(fifo, fifo->head, n);
  if (n >= rt_fifo_readable_payload(fifo)) {
    fifo->read_empty = 1;
    if (fifo->write_pos == fifo->tail) {
      fifo->empty = 1;
    }
    // dequeueing past write_pos is not allowed
    target = fifo->write_pos;
    // printf("fifo now empty. exiting.\n");
  }
  while (fifo->head != target) {
    fifo->queue[fifo->head] = 0.;
    fifo->head              = rt_fifo_new_pos(fifo, fifo->head, 1);
  }
}

void rt_fifo_dequeue_staggered(rt_fifo fifo, rt_real *dest, int n, int advance)
{
  rt_fifo_read(fifo, dest, n);
  rt_fifo_dequeue(fifo, advance);
}

inline size_t rt_fifo_payload(rt_fifo fifo)
{
  size_t payload = rt_fifo_get_diff(fifo, fifo->head, fifo->tail);
  if (payload == 0 && !(fifo->empty)) {
    payload = fifo->size;
  }
  return payload;
}

inline size_t rt_fifo_readable_payload(rt_fifo fifo)
{
  size_t wpayload = rt_fifo_get_diff(fifo, fifo->head, fifo->write_pos);
  if (wpayload == 0 && !(fifo->read_empty)) {
    wpayload = fifo->size;
  }
  return wpayload;
}

rt_fifo rt_fifo_destroy(rt_fifo fifo)
{
  free(fifo->queue);
  free(fifo);
  return (rt_fifo)NULL;
}
