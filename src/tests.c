#include "phasevoc.h"

typedef struct WAV {
  char   headers[44];
  float *data[2];
  size_t data_len;
} WAV;

WAV  read_from_wav(const char *filename, const stft_params_t *p);

void write_to_wav(const char *filename, WAV *wav);

int  main()
{
  stft_params_t p = {1 << 20, 2048, 4.f};
  rtst_calculate_parameters(&p);
  WAV wav = read_from_wav("in.wav", &p);
  write_to_wav("out.wav", &wav);
  return 1;
}

WAV read_from_wav(const char *filename, const stft_params_t *p)
{
  WAV wav;
  wav.data_len      = p->block_size;
  FILE *file        = fopen(filename, "rb");
  wav.data[0]       = (float *)malloc(sizeof(float) * p->block_size);
  wav.data[1]       = (float *)malloc(sizeof(float) * p->block_size);
  int16_t *dataread = (int16_t *)malloc(sizeof(int16_t) * p->block_size);

  fread(wav.headers, 1, 44, file);
  int readsize = 4096;
  for (size_t i = 0; i < p->block_size; i += readsize) {
    fread(dataread + i, sizeof(int16_t), readsize, file);
  }
  for (size_t i = 0; i < p->block_size; i++) {
    wav.data[i % 2][i / 2] = (float)dataread[i];
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