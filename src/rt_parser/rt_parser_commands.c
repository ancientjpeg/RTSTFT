#include "../rt_globals.h" /* for intellisense */
#include "../rtstft.h"

float rt_parser_float_from_numeric(rt_arg_t *arg)
{
  if (arg->arg_type == RT_INT_ARG) {
    return (float)arg->raw_arg.i;
  }
  else if (arg->arg_type == RT_FLOAT_ARG) {
    return arg->raw_arg.f;
  }
  return RT_REAL_ERR;
}
int rt_parser_int_from_numeric(rt_arg_t *arg)
{
  if (arg->arg_type == RT_FLOAT_ARG) {
    return (float)arg->raw_arg.i;
  }
  else if (arg->arg_type == RT_INT_ARG) {
    return arg->raw_arg.i;
  }
  return RT_INT_ERR;
}