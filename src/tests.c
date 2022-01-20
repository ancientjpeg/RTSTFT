#include "rtstft.h"
#include "wavfile.h"
#include <time.h>
#include <unistd.h>
#define start_timer(t) t = clock()
#define stop_timer(t)                                                          \
  printf("\ntime taken: %.5fms\n",                                             \
         ((double)(clock() - t) / CLOCKS_PER_SEC) * 1000.)

void printReals(FILE *stream, rt_real *arr, size_t len)
{
  fprintf(stream, "[ \n");
  for (size_t i = 0; i < len; i++) {
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
  size_t    block_size  = 1024;
  int       buffer_size = 128;
  rt_params p           = rt_init(block_size, 256, 4.f, 44100, buffer_size);
  WAV       wav         = read_from_wav("in.wav", p->block_size);
  start_timer(t);
  for (int f = 0; f < block_size / buffer_size; f++) {
    rt_fifo_enqueue(p->in, wav.data[0], buffer_size);
    // printReals(stdout, rt_fifo_get_head_ptr(p->in), p->in->size);
    rt_cycle(p);
    // printReals(stdout, rt_fifo_get_head_ptr(p->in), p->in->size);
    // TODO assemble frame through out fifo back to wav
    // latency ??
    // manage control rates between the buffers
  }
  for (size_t i = 0; i < p->block_size; i++) {
    wav.data[0][i] = p->block->frames[0][i];
    wav.data[1][i] = p->block->frames[0][i];
  }
  stop_timer(t);
  write_to_wav("out.wav", &wav);
  rt_clean(p);
  return 0;
}
