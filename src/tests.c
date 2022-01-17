#include "rtstft.h"
#include "wavfile.h"
#include <time.h>

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
  rt_params p = rt_init(1 << 20, 1024, 4.f);
  int       a = 20, b = 12;
  rt_real  *x = fftw_alloc_real(a);
  rt_real  *y = fftw_alloc_real(b);
  for (int i = 0; i < a; i++) {
    x[i] = sin((float)i / 3);
  }
  FILE *json = toJSON();
  printReals(json, x, a);
  rt_lerp(x, a, y, b);
  printReals(json, y, b);
  json = closeJSON(json);
  rt_clean(p);
  return 0;
}
