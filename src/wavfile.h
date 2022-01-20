#include "rtstft.h"

typedef struct WAV {
  char     headers[44];
  rt_real *data[2];
  size_t   data_len;
} WAV;

WAV  read_from_wav(const char *filename, const size_t size);

void write_to_wav(const char *filename, WAV *wav);