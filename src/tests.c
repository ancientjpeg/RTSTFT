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
  fprintf(stream, "[ ");
  for (size_t i = 0; i < len; i++) {
    fprintf(stream, "%f, ", arr[i]);
  }
  fprintf(stream, "],\n");
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
  rt_params p = rt_init(1 << 20, 1024, 4.f);
  for (int i = 0; i < a; i++) {
    x[i] = sin((float)i / 3);
  }
  FILE *json = toJSON();
  start_timer(t);
  stop_timer(t);
  rt_clean(p);
  return 0;
}
