/**
 * @file rt_fifo.h
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RT_FIFO_H
#define RT_FIFO_H

#include "rt_globals.h"

#define rt_fifo_head_ptr(f) ((f)->queue + (f)->head)
#define rt_fifo_write_ptr(f) ((f)->queue + (f)->head)
#define rt_fifo_tail_ptr(f) ((f)->queue + (f)->tail)
#define rt_fifo_new_pos(f, i, n) ((rt_uint)((ldiv(((i) + (n)), (f)->len)).rem))
#define rt_fifo_get_diff(f, start, end)                                        \
  ((start) <= (end) ? (end) - (start) : (f)->len - ((start) - (end)))

typedef struct RT_FIFO {
  rt_real *queue;
  rt_uint  len, head, write_pos, tail;
  char     empty, read_empty;
} rt_fifo_t;

typedef rt_fifo_t *rt_fifo;

#endif
