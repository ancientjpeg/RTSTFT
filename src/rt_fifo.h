#ifndef RT_FIFO
#define RT_FIFO
#include "rt_globals.h"

typedef struct RTSTFT_FIFO {
  float       *queue;
  unsigned int num_frames;
  size_t       length;
} rt_fifo;

#endif