#include "rtstft.h"

rt_fifo rt_fifo_init(rt_uint len)
{
  rt_fifo fifo     = (rt_fifo)malloc(sizeof(rt_fifo_t));
  fifo->head       = 0;
  fifo->write_pos  = 0;
  fifo->tail       = 0;
  fifo->empty      = 1;
  fifo->read_empty = 1;
  fifo->len        = len;
  fifo->queue      = (rt_real *)calloc(sizeof(rt_real), fifo->len);
  return fifo;
}
void rt_fifo_enqueue(rt_fifo fifo, rt_real *data, int n)
{
  rt_fifo_enqueue_staggered(fifo, data, n, n);
}

void rt_fifo_enqueue_one(rt_fifo fifo, rt_real *data)
{
  if (rt_fifo_payload(fifo) == fifo->len) {
    fprintf(stderr, "Cannot enqueue sample. FIFO is full.");
    exit(1);
  }
  fifo->empty      = 0;
  fifo->read_empty = 0;
  fifo->queue[fifo->write_pos] += *data;
  char wasTail    = fifo->write_pos == fifo->tail ? 1 : 0;
  fifo->write_pos = rt_fifo_new_pos(fifo, fifo->write_pos, 1);
  fifo->tail      = wasTail ? fifo->write_pos : fifo->tail;
}

void rt_fifo_enqueue_staggered(rt_fifo fifo, rt_real *data, int n, int advance)
{

  rt_uint payload     = rt_fifo_payload(fifo);
  rt_uint new_payload = payload + n;
  if (new_payload >= fifo->len) {
    n       = fifo->len - payload;
    advance = advance > n ? n : advance;
    if (new_payload > fifo->len) {
      fprintf(stderr, "attempted to overfill FIFO. %i\n", n);
      exit(1);
    }
  }
  fifo->empty         = 0;
  fifo->read_empty    = 0;
  rt_uint write_init  = fifo->write_pos;
  char    passed_tail = 0;
  rt_uint i;
  for (i = 0; i < n; i++) {
    fifo->queue[fifo->write_pos] += data[i];
    if (fifo->write_pos == fifo->tail) {
      passed_tail = 1;
    }
    fifo->write_pos = rt_fifo_new_pos(fifo, fifo->write_pos, 1);
  }
  fifo->tail = passed_tail ? fifo->write_pos : fifo->tail;
  if (n != advance) {
    fifo->write_pos = rt_fifo_new_pos(fifo, write_init, advance);
  }
}

void rt_fifo_read(rt_fifo fifo, rt_real *dest, int n)
{
  if (fifo->read_empty) {
    fprintf(stderr, "Error: Nothing to read.");
    exit(1);
  }
  else if (n > rt_fifo_readable(fifo)) {
    fprintf(stderr, "Error: cannot read beyond the current write pointer.\n");
    exit(1);
  }
  rt_uint i;
  for (i = 0; i < n; i++) {
    rt_uint index = i != 0 ? rt_fifo_new_pos(fifo, fifo->head, i) : fifo->head;
    dest[i]       = fifo->queue[index];
  }
}

void rt_fifo_dequeue(rt_fifo fifo, int n)
{
  if (fifo->empty) {
    fprintf(stderr, "Error: Nothing to dequeue.");
    exit(1);
  }
  int target = rt_fifo_new_pos(fifo, fifo->head, n);
  if (n >= rt_fifo_readable(fifo)) {
    fifo->read_empty = 1;
    if (fifo->write_pos == fifo->tail) {
      fifo->empty = 1;
    }
    /* dequeueing past write_pos is not allowed */
    target = fifo->write_pos;
    // printf("fifo now empty. exiting.\n");
  }
  while (fifo->head != target) {
    fifo->queue[fifo->head] = 0.;
    fifo->head              = rt_fifo_new_pos(fifo, fifo->head, 1);
  }
}

void rt_fifo_dequeue_one(rt_fifo fifo, rt_real *dest)
{
  if (fifo->empty) {
    fprintf(stderr, "Error: Nothing to dequeue.");
    exit(1);
  }
  if (dest != NULL) {
    if (fifo->read_empty) {
      fprintf(stderr, "Error: Nothing to read. ");
      exit(1);
    }
    *dest = fifo->queue[fifo->head];
  }
  fifo->queue[fifo->head] = 0.;
  fifo->head              = rt_fifo_new_pos(fifo, fifo->head, 1);
  if (fifo->head == fifo->write_pos) {
    fifo->read_empty = 1;
    fifo->empty      = fifo->head == fifo->tail ? 1 : 0;
  }
}

void rt_fifo_dequeue_staggered(rt_fifo fifo, rt_real *dest, int n, int advance)
{
  rt_fifo_read(fifo, dest, n);
  rt_fifo_dequeue(fifo, advance);
}

rt_uint rt_fifo_payload(rt_fifo fifo)
{
  rt_uint payload = rt_fifo_get_diff(fifo, fifo->head, fifo->tail);
  if (payload == 0 && !(fifo->empty)) {
    payload = fifo->len;
  }
  return payload;
}

rt_uint rt_fifo_readable(rt_fifo fifo)
{
  rt_uint wpayload = rt_fifo_get_diff(fifo, fifo->head, fifo->write_pos);
  if (wpayload == 0 && !(fifo->read_empty)) {
    wpayload = fifo->len;
  }
  return wpayload;
}

rt_fifo rt_fifo_destroy(rt_fifo fifo)
{
  free(fifo->queue);
  free(fifo);
  return (rt_fifo)NULL;
}
