#include "phasevoc.h"

typedef struct WAV {
  char   headers[44];
  float *data[2];
  size_t data_len;
} WAV;

WAV  read_from_wav(const char *filename, const rt_params_t *p);

void write_to_wav(const char *filename, WAV *wav);