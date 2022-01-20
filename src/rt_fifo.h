#ifndef RT_FIFO_H
#define RT_FIFO_H

#include "rt_globals.h"

// WHY IS THIS MAKING INTELLISENSE SHIT ITSELF????
// #define rt_fifo_get_head_ptr(f) (f->queue + f->head)
// #define rt_fifo_get_tail_ptr(f) (f->queue + f->tail)

typedef struct RT_FIFO {
  rt_real *queue;
  size_t   size, head, tail;
  char     empty;
} rt_fifo_t;

typedef rt_fifo_t *rt_fifo;

rt_fifo            rt_fifo_init(size_t size);
void               rt_fifo_enqueue(rt_fifo fifo, rt_real *data, int n);
void rt_fifo_enqueue_staggered(rt_fifo fifo, rt_real *data, int n, int advance);
void rt_fifo_read(rt_fifo fifo, rt_real *dest, int n);
void rt_fifo_dequeue(rt_fifo fifo, int n);
void rt_fifo_dequeue_staggered(rt_fifo fifo, rt_real *dest, int n, int advance);
size_t   rt_fifo_get_payload(rt_fifo fifo);

rt_fifo  rt_fifo_destroy(rt_fifo fifo);
rt_real *rt_fifo_get_head_ptr(rt_fifo fifo);
rt_real *rt_fifo_get_tail_ptr(rt_fifo fifo);

#endif