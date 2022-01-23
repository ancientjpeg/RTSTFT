#ifndef RT_FIFO_H
#define RT_FIFO_H

#include "rt_globals.h"

#define rt_fifo_head_ptr(f) ((f)->queue + (f)->head)
#define rt_fifo_write_ptr(f) ((f)->queue + (f)->head)
#define rt_fifo_tail_ptr(f) ((f)->queue + (f)->tail)
#define rt_fifo_new_pos(f, i, n) (((i) + (n)) % (f)->size)
#define rt_fifo_get_diff(f, start, end)                                        \
  ((start) <= (end) ? (end) - (start) : (f)->size - ((start) - (end)))

typedef struct RT_FIFO {
  rt_real *queue;
  size_t   size, head, write_pos, tail;
  char     empty, read_empty;
} rt_fifo_t;

typedef rt_fifo_t *rt_fifo;

rt_fifo            rt_fifo_init(size_t size);
void               rt_fifo_enqueue(rt_fifo fifo, rt_real *data, int n);
void rt_fifo_enqueue_staggered(rt_fifo fifo, rt_real *data, int n, int advance);
void rt_fifo_read(rt_fifo fifo, rt_real *dest, int n);
void rt_fifo_dequeue(rt_fifo fifo, int n);
void rt_fifo_dequeue_staggered(rt_fifo fifo, rt_real *dest, int n, int advance);
size_t  rt_fifo_payload(rt_fifo fifo);
size_t  rt_fifo_readable_payload(rt_fifo fifo);

rt_fifo rt_fifo_destroy(rt_fifo fifo);

#endif