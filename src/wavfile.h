/**
 * @file wavfile.h
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "rtstft.h"

typedef struct WAV {
  char     headers[44];
  rt_real *data[2];
  rt_uint  data_len;
} WAV;

WAV  read_from_wav(const char *filename, const rt_uint size);

void write_to_wav(const char *filename, WAV *wav);