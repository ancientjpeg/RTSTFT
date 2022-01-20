#include "wavfile.h"
#include <unistd.h>
WAV read_from_wav(const char *filename, const size_t size)
{
  WAV wav;
  wav.data_len = size;
  char dir[100];
  getcwd(dir, 100);
  printf("dir: %s\n", dir);
  FILE *file        = fopen(filename, "rb");
  wav.data[0]       = (rt_real *)malloc(sizeof(rt_real) * wav.data_len);
  wav.data[1]       = (rt_real *)malloc(sizeof(rt_real) * wav.data_len);
  int16_t *dataread = (int16_t *)malloc(sizeof(int16_t) * wav.data_len);

  fread(wav.headers, 1, 44, file);
  int readsize = 4096;
  for (size_t i = 0; i < wav.data_len; i += readsize) {
    fread(dataread + i, sizeof(int16_t), readsize, file);
  }
  for (size_t i = 0; i < wav.data_len; i++) {
    wav.data[i % 2][i / 2] = (rt_real)dataread[i];
  }
  int32_t filesize                 = 44 + wav.data_len;
  *((int32_t *)(wav.headers + 4))  = filesize;
  *((int32_t *)(wav.headers + 40)) = wav.data_len;

  free(dataread);
  fclose(file);

  return wav;
}

void write_to_wav(const char *filename, WAV *wav)
{
  FILE    *file      = fopen(filename, "wb");
  int16_t *datawrite = (int16_t *)malloc(sizeof(int16_t) * wav->data_len);

  int      writesize = 4096;
  for (size_t i = 0; i < wav->data_len; i++) {
    datawrite[i] = (int16_t)wav->data[i % 2][i / 2];
  }
  fwrite(wav->headers, 1, 44, file);
  for (size_t i = 0; i < wav->data_len; i += writesize) {
    fwrite(datawrite + i, sizeof(int16_t), writesize, file);
  }

  free(wav->data[0]);
  free(wav->data[1]);
  fclose(file);
}
