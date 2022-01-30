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
  rt_uint by = 16;
  for (i = 0; i < len; i++) {
    if (i % by == by - 1) {
      fprintf(stream, "    ");
    }
    fprintf(stream, "%.1f, ", arr[i]);
    if (i % by == by - 1 && i != len - 1) {
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
  rt_uint   block_size   = 1 << 21;
  rt_uint   buffer_size  = 1 << 12;
  rt_uint   frame_size   = 1 << 12;
  float     scale_factor = 2.;
  rt_params p[2];
  rt_uint   i, f;
  for (i = 0; i < 2; i++) {
    p[i] = rt_init(frame_size, 8, buffer_size, 44100.f, scale_factor);
  }
  WAV     wav = read_from_wav("in.wav", block_size);
  rt_real temp_null;
  rt_uint latency_size = frame_size;
  /*   for (i = 0; i < block_size; i++) {
      if (i >= latency_size && i < block_size - latency_size) {
        wav.data[1][i] = wav.data[0][i - latency_size];
      }
      else {
        wav.data[1][i] = 0.;
      }
      wav.data[0][i] *= 1.5;
      wav.data[1][i] *= 0.6;
    } */

  start_timer(t);
  for (i = 0; i < 2; i++) {
    for (f = 0; f < block_size; f += buffer_size) {
      rt_cycle(p[i], wav.data[i] + f, buffer_size);
    }
  }
  stop_timer(t);

  write_to_wav("out.wav", &wav);
  rt_clean(p[0]);
  return 0;
}
