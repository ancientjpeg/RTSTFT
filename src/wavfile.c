#include "wavfile.h"
#include <unistd.h>
WAV read_from_wav(const char *filename, const rt_uint size)
{
  WAV wav;
  wav.data_len         = size;
  rt_uint raw_data_len = sizeof(int16_t) * wav.data_len * 2;
  wav.data[0]          = (rt_real *)malloc(sizeof(rt_real) * wav.data_len);
  wav.data[1]          = (rt_real *)malloc(sizeof(rt_real) * wav.data_len);
  int16_t *dataread    = (int16_t *)malloc(raw_data_len * 2);

  FILE    *file        = fopen(filename, "rb");
  if (file == NULL) {
    exit(15);
  }
  fread(wav.headers, 1, 44, file);
  int     readsize = 4096;
  rt_uint i;
  for (i = 0; i < wav.data_len; i += readsize) {
    if (i + readsize > raw_data_len) {
      readsize = (raw_data_len) % i;
    }
    fread(dataread + i, sizeof(int16_t), readsize, file);
  }
  for (i = 0; i < wav.data_len; i++) {
    wav.data[i % 2][i / 2] = (rt_real)dataread[i];
  }
  int32_t filesize                 = 44 + (wav.data_len * sizeof(rt_real));
  *((int32_t *)(wav.headers + 4))  = filesize;
  *((int32_t *)(wav.headers + 40)) = wav.data_len;

  free(dataread);
  fclose(file);

  return wav;
}

void write_to_wav(const char *filename, WAV *wav)
{
  int16_t *datawrite = (int16_t *)malloc(sizeof(int16_t) * wav->data_len);

  int      writesize = 4096;
  rt_uint  i;
  for (i = 0; i < wav->data_len; i++) {
    datawrite[i] = (int16_t)wav->data[i % 2][i / 2];
  }
  FILE *file = fopen(filename, "wb");
  if (!file) {
    return;
  }
  fwrite(wav->headers, 1, 44, file);
  for (i = 0; i < wav->data_len; i += writesize) {
    fwrite(datawrite + i, sizeof(int16_t), writesize, file);
  }

  free(wav->data[0]);
  free(wav->data[1]);
  fclose(file);
}
