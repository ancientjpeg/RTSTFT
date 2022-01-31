#include "wavfile.h"

WAV read_from_wav(const char *filename, const rt_uint size)
{
  if (sizeof(int) != 4 || sizeof(short) != 2) {
    fprintf(
        stderr,
        "I'm very lazy but short needs to be 2 bytes and int needs to be 4.\n");
    exit(1);
  }
  WAV wav;
  wav.data_len         = size;
  wav.data[0]          = (rt_real *)malloc(sizeof(rt_real) * wav.data_len);
  wav.data[1]          = (rt_real *)malloc(sizeof(rt_real) * wav.data_len);
  rt_uint raw_data_len = sizeof(short) * wav.data_len * 2;
  short  *dataread     = (short *)malloc(raw_data_len);

  FILE   *file         = fopen(filename, "rb");
  if (file == NULL) {
    exit(15);
  }
  fread(wav.headers, 1, 44, file);
  int     readsize = 4096;
  rt_uint i;
  for (i = 0; i < wav.data_len * 2; i += readsize) {
    if (i + readsize > raw_data_len) {
      readsize = (raw_data_len) % i;
    }
    fread(dataread + i, sizeof(short), readsize, file);
  }
  for (i = 0; i < wav.data_len * 2; i++) {
    wav.data[i % 2][i / 2] = (rt_real)dataread[i];
  }

  int filesize                 = 44 + (raw_data_len);
  *((int *)(wav.headers + 4))  = 36 + raw_data_len;
  *((int *)(wav.headers + 40)) = raw_data_len;

  free(dataread);
  fclose(file);

  return wav;
}

void write_to_wav(const char *filename, WAV *wav)
{
  rt_uint raw_data_len = sizeof(short) * wav->data_len * 2;
  short  *datawrite    = (short *)malloc(raw_data_len);

  rt_uint i;
  for (i = 0; i < wav->data_len * 2; i++) {
    datawrite[i] = (short)wav->data[i % 2][i / 2];
  }
  FILE *file = fopen(filename, "wb");
  if (!file) {
    return;
  }
  fwrite(wav->headers, 1, 44, file);
  int writesize = 4096;
  for (i = 0; i < raw_data_len; i += writesize) {
    fwrite(datawrite + i, writesize, sizeof(short), file);
  }

  free(wav->data[0]);
  free(wav->data[1]);
  fclose(file);
}
