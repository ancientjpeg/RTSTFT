#include "../rt_globals.h" /* for intellisense */
#include "../rtstft.h"

float rt_parser_float_from_numeric(rt_token_t *arg)
{
  if (arg->token_flavor == RT_CMD_INT_T) {
    return (float)arg->raw_arg.i;
  }
  else if (arg->token_flavor == RT_CMD_FLOAT_T) {
    return arg->raw_arg.f;
  }
  return RT_REAL_ERR;
}
int rt_parser_int_from_numeric(rt_token_t *arg)
{
  if (arg->token_flavor == RT_CMD_FLOAT_T) {
    return (float)arg->raw_arg.i;
  }
  else if (arg->token_flavor == RT_CMD_INT_T) {
    return arg->raw_arg.i;
  }
  return RT_INT_ERR;
}