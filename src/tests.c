/**
 * @file tests.c
 * @author Jackson Kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief
 * @version 0.1a1
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "rtstft.h"
#include "wavfile.h"
#include <time.h>
#include <unistd.h>
#define start_timer(t) t = clock()
#define stop_timer(t)                                                          \
  printf("\nprocessing time: %.5fms\n",                                        \
         ((double)(clock() - t) / CLOCKS_PER_SEC) * 1000.)

void  printReals(FILE *stream, rt_real *arr, rt_uint len);
FILE *toJSON();
FILE *closeJSON(FILE *json);
void  test_audio();
void  test_parse();

int   main()
{
  // test_audio();
  test_parse();
  return 0;
}

void test_audio()
{
  time_t  t;
  rt_uint buffer_size = 1UL << 14;
  rt_uint frame_size  = 1UL << 14;
  rt_uint block_size  = 1 << 20;
  /*   float     scale_factor = pow(2, (float)12. / 12.); */
  rt_uint   f;
  rt_params p   = rt_init(2, frame_size, buffer_size, 8, 0, 44100.f);
  WAV       wav = read_from_wav("in.wav", block_size);
  /*   rt_uint i;
    for (i = 0; i < block_size; i++) {
      double val     = sin((double)i / 44100. * 2 * M_PI * 500.);
      wav.data[0][i] = val * 0.2;
      wav.data[1][i] = val * 0.2;
    } */

  start_timer(t);
  for (f = 0; f < block_size; f += buffer_size) {
    rt_real this_scale = ((float)f / block_size * .6) + 1.0;
    this_scale         = this_scale > 1.334 ? 1.334 : this_scale;
    /*     this_scale         = 1.256; */
    rt_set_scale_factor(p, this_scale);
    rt_start_cycle(p);
    rt_cycle_offset(p, wav.data, 2, buffer_size, f);
    rt_end_cycle(p);
  }
  stop_timer(t);

  write_to_wav("out.wav", &wav);
  rt_clean(p);
}

void test_parse()
{
  rt_params p      = rt_init(2, 512, 512, 8, 0, 44100.f);
  int       status = rt_parse_and_execute(p, "limit -sx 5 25-30 -6");
  if (status) {
    printf("%s\n", p->parser.error_msg_buffer);
  }
  rt_clean(p);
}

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