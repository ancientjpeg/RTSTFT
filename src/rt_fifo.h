#ifndef RT_FIFO_H
#define RT_FIFO_H

#include "rt_globals.h"

typedef struct RT_FIFO {
  rt_real *queue;
  size_t   size, payload, head, tail;
} rt_fifo_t;

typedef rt_fifo_t *rt_fifo;

#endif