/**
 * @file rt_parser_exec.c
 * @author jackson kaplan (jacksonkaplan@alum.calarts.edu)
 * @brief this file is ugly as hell I am so sorry
 * @version 0.1
 * @date 2022-03-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "../rtstft.h"

int rt_parser_execute_gain_gate_limit(rt_params p);
int rt_parser_execute_gain(void *params_ptr)
{
  rt_parser_execute_gain_gate_limit((rt_params)params_ptr);
  return 1;
}
int rt_parser_execute_gate(void *params_ptr)
{
  rt_parser_execute_gain_gate_limit((rt_params)params_ptr);
  return 1;
}
int rt_parser_execute_limit(void *params_ptr)
{
  rt_parser_execute_gain_gate_limit((rt_params)params_ptr);
  return 1;
}
int rt_parser_execute_rebase(void *params_ptr)
{
  rt_params p = (rt_params)params_ptr;
  return 1;
}
int rt_parser_execute_transpose(void *params_ptr)
{
  rt_params p = (rt_params)params_ptr;
  return 1;
}
int rt_parser_execute_gain_gate_limit(rt_params p)
{
  rt_command_t *cmd         = &(p->parser.command);
  rt_uint       token_index = 0, bin0, binN, i;
  rt_real       new_level_0_1;
  rt_real       value0 = -1.f, valueN = -1.f, curve_pow = -5.f;
  rt_token_t    token_curr;
  unsigned char values_in_db = 0;
  for (i = 0; i < RT_CMD_MAX_OPTS; i++) {
    switch (cmd->options[i].flag) {
    case '\0':
      break;
    case 'b':
      values_in_db = 1;
      break;
    case 'e':
      break;
    }
  }

  // /* begin range checks */
  // if (bin0 > rt_manip_len || binN > rt_manip_len || binN < bin0) {
  //   /* bin0 < 0 for uint is just a big number */
  //   sprintf(p->parser.error_msg_buffer, "Improper bin range: %i-%i", bin0,
  //           binN);
  //   return 6;
  // }

  // if (p->parser.active_cmd_def->manip_flavor == RT_MANIP_GAIN && value0
  // > 2.f) {
  //   sprintf(p->parser.error_msg_buffer, "Too much gain!: %f", new_level_0_1);
  //   return 6;
  // }
  // else if (value0 > 1.f || value0 < 0.f) {
  //   sprintf(p->parser.error_msg_buffer, "Badly ranged manip value: %f",
  //           new_level_0_1);
  //   return 6;
  // }
  /* only mono for now */
  if (curve_pow == -1.f) {
    // rt_manip_set_bins(p, p->chans[0], p->parser.active_cmd_def->manip_flavor,
    //                   bin0, binN, new_level_0_1);
  }
  else {
  }
  return 0;
}
