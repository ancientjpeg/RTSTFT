#include "../rt_globals.h" /* for intellisense */
#include "../rtstft.h"

int   rt_parser_parse_curve();
int   rt_parser_int_from_numeric(rt_lexed_arg_t *arg);
float rt_parser_float_from_numeric(rt_lexed_arg_t *arg);
int   rt_parser_parse_gain(rt_params p)
{
  rt_uint         i = 1, bin0 = RT_UINT_FALSE, binN = RT_UINT_FALSE;
  rt_real         gain = -1.;
  rt_uint         cmd_args_parsed;
  rt_real         curve0, curveN, curve_pow;
  rt_lexed_arg_t *arg_curr;
  do {
    arg_curr = p->parser.token_buffer + i;
    if (arg_curr->arg_type == RT_PARAM_ARG) {
    }
    else {
      if (bin0 == RT_UINT_FALSE) {
      }
      else if (binN == RT_UINT_FALSE) {
      }
      else if (gain == -1.) {
        gain = rt_parser_float_from_numeric(arg_curr);
      }
      else {
        return -1;
      }
      cmd_args_parsed++;
    }
  } while (p->parser.token_buffer[++i].arg_type != RT_UNDEFINED_ARG);
  rt_uint chans = p->manip_multichannel ? p->num_chans : 1;
  for (i = 0; i < chans; i++) {
    rt_manip_set_bins(p, p->chans[i], RT_MANIP_GAIN, bin0, binN, gain);
  }
  return 0;
}
int   rt_parser_parse_gate(rt_params p) {}
int   rt_parser_parse_limit(rt_params p) {}

float rt_parser_float_from_numeric(rt_lexed_arg_t *arg)
{
  if (arg->arg_type == RT_INT_ARG) {
    return (float)arg->raw_arg.i;
  }
  else if (arg->arg_type == RT_FLOAT_ARG) {
    return arg->raw_arg.f;
  }
  return RT_REAL_ERR;
}
int rt_parser_int_from_numeric(rt_lexed_arg_t *arg)
{
  if (arg->arg_type == RT_FLOAT_ARG) {
    return (float)arg->raw_arg.i;
  }
  else if (arg->arg_type == RT_INT_ARG) {
    return arg->raw_arg.i;
  }
  return RT_INT_ERR;
}