#include "phasevoc.h"

double tc(clock_t t) { return ((double)t) / CLOCKS_PER_SEC; }

void   full_print(fftw_complex *stft_out, stft_params_t p);

int    main()
{
  const char *infile = "x.wav";
  FILE       *file   = fopen(infile, "rb");
  fseek(file, 0, SEEK_END);
  size_t fSize = ftell(file);
  rewind(file);
  size_t        blocksize = 1U << 19;
  size_t        framesize = 4096;
  size_t        by        = 4096;
  size_t        num_reads = blocksize / by;
  double        sixteen   = 1U << 16;

  stft_params_t p         = stft_params(blocksize, framesize, 4.f);
  float        *buffer    = malloc(p.block_size * sizeof(float));
  int16_t      *ibuf      = malloc(p.block_size * sizeof(int16_t));
  fftw_complex *stft_out  = stft_alloc_rfft_buffer(p);
  fftw_plan    *plans     = create_plans(stft_out, p);
  fftw_plan    *plans_rev = create_plans_rev(stft_out, p);
  clock_t       t         = clock();

  char          headers[44];

  printf("%f alloc seconds\n", tc(clock() - t));
  t = clock();

  fread(headers, 1, 44, file);
  for (size_t i = 0; i < num_reads; i++) {
    for (int j = 0; j < by; j++) {
      size_t index = j + (by * i);
      fseek(file, sizeof(int16_t), SEEK_CUR);
      fread(ibuf + index, sizeof(int16_t), 1, file);
      buffer[index] = ibuf[index] / sixteen;
    }
  }
  fclose(file);
  file = NULL;

  stft_copy_float_to_buffer(stft_out, buffer, p);
  printf("%f assign seconds\n", tc(clock() - t));
  t = clock();
  STFT(stft_out, p, plans);
  printf("%f STFT seconds\n", tc(clock() - t));
  // full_print(stft_out, p);

  t = clock();
  ISTFT(stft_out, p, plans_rev);
  printf("%f ISTFT seconds\n", tc(clock() - t));
  stft_collapse_istft_to_floats(buffer, stft_out, p);
  // full_print(stft_out, p);

  file = fopen("out.wav", "wb");
  fwrite(headers, 1, 44, file);
  for (size_t i = 0; i < num_reads; i++) {
    for (int j = 0; j < by; j++) {
      size_t index = j + (by * i);
      ibuf[index]  = (1 << 2) * buffer[index];
    }
    fwrite(ibuf + (i * by), sizeof(int16_t), by, file);
  }
  fseek(file, 22, SEEK_SET);
  int16_t chan = 1;
  fwrite(&chan, sizeof(chan), 1, file);

  fseek(file, 32, SEEK_SET);
  int16_t align = 2;
  fwrite(&align, sizeof(align), 1, file);
  fseek(file, 28, SEEK_SET);
  int32_t byterate = 88200;
  fwrite(&byterate, sizeof(byterate), 1, file);
  fseek(file, 40, SEEK_SET);
  int32_t size = blocksize;
  fwrite(&size, sizeof(size), 1, file);
  fclose(file);
  file = NULL;

  // stft_cleanup(stft_out, p, plans, plans_rev);
  return 0;
}

void full_print(fftw_complex *stft_out, stft_params_t p)
{
  for (int i = 0; i < p.num_frames / 2; i++) {
    for (int j = 0; j < (p.frame_size / 2 + 1); j++) {
      size_t this_index = i * (p.frame_size / 2 + 1) + j;
      double a          = stft_out[this_index][0];
      double b          = stft_out[this_index][1];
      double angle      = atan2(b, a);
      double amp        = sqrt(a * a + b * b);
      printf("index %3d %3d: out: %4.4f %4.4f    angle: %3.3f    amp: %3.3f\n",
             i, j, stft_out[this_index][0], stft_out[this_index][1], angle,
             amp);
    }
  }
}