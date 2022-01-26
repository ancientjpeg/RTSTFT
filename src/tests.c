#include "rtstft.h"
#include "wavfile.h"
#include <time.h>
#include <unistd.h>
#define start_timer(t) t = clock()
#define stop_timer(t)                                                          \
  printf("\ntime taken: %.5fms\n",                                             \
         ((double)(clock() - t) / CLOCKS_PER_SEC) * 1000.)

void printReals(FILE *stream, rt_real *arr, rt_uint len)
{
  fprintf(stream, "[ \n");
  rt_uint i;
  for (i = 0; i < len; i++) {
    if (i % 16 == 15) {
      fprintf(stream, "    ");
    }
    fprintf(stream, "%.1f, ", arr[i]);
    if (i % 16 == 15 && i != len - 1) {
      fprintf(stream, "\n");
    }
  }
  fprintf(stream, "\n],\n");
}

FILE *toJSON()
{
  FILE *json = fopen("out.json", "w");
  fprintf(json, "{\n  \"data\": [\n");
  return json;
}

FILE *closeJSON(FILE *json)
{
  fprintf(json, "]\n}\n");
  fclose(json);
  return (FILE *)NULL;
}

int main()
{
  time_t    t;
  rt_uint   block_size  = 1 << 18;
  rt_uint   buffer_size = 1 << 13;
  rt_uint   frame_size  = 1 << 10;
  rt_params p[2];
  p[0]        = rt_init(frame_size, 4, buffer_size, 44100.f);
  p[1]        = rt_init(frame_size, 4, buffer_size, 44100.f);
  WAV     wav = read_from_wav("in.wav", block_size);
  rt_real temp_null;
  rt_uint i, f;
  printReals(stdout, wav.data[0], 64);
  printReals(stdout, wav.data[1], 64);
  start_timer(t);
  for (i = 0; i < 2; i++) {
    for (f = 0; f < block_size; f += buffer_size) {
      rt_cycle(p[i], wav.data[i] + f, buffer_size);
    }
  }
  stop_timer(t);
  printReals(stdout, wav.data[0] + frame_size * 2, 64);
  printReals(stdout, wav.data[1] + frame_size * 2, 64);

  write_to_wav("out.wav", &wav);
  rt_clean(p[0]);
  return 0;
}
