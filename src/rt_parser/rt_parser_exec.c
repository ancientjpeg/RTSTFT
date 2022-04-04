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

rt_real rt_float_from_any_numeric_token(rt_token_t *token)
{
  if (token->token_flavor == RT_CMD_INT_T) {
    return (float)token->raw_arg.i;
  }
  return token->raw_arg.f;
}

int rt_parser_execute_gain_gate_limit(rt_params p);
int rt_parser_execute_gain(void *params_ptr)
{
  return rt_parser_execute_gain_gate_limit((rt_params)params_ptr);
}
int rt_parser_execute_gate(void *params_ptr)
{
  return rt_parser_execute_gain_gate_limit((rt_params)params_ptr);
}
int rt_parser_execute_limit(void *params_ptr)
{
  return rt_parser_execute_gain_gate_limit((rt_params)params_ptr);
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

int rt_parser_exec_check_level(rt_params p, rt_real val)
{
  if (val > 1.0f || val < 0.f) {
    if (p->parser.active_cmd_def->manip_flavor == RT_MANIP_GAIN && val > 2.f) {
      sprintf(p->parser.error_msg_buffer, "Too much gain!: %f", val);
      return 11;
    }
    else {
      sprintf(p->parser.error_msg_buffer, "Badly ranged manip value: %f", val);
      return 12;
    }
  }
  return 0;
}

int rt_parser_execute_gain_gate_limit(rt_params p)
{
  rt_command_t              *cmd  = &(p->parser.command);
  const rt_command_define_t *def  = p->parser.active_cmd_def;
  rt_uint                    bin0 = -1, binN = -1, i;
  rt_real                    value0 = -1.f, valueN = -1.f, curve_pow = -100.f;
  unsigned char              values_in_db = 0, got_curve_opt = 0;
  for (i = 0; i < RT_CMD_MAX_OPTS; i++) {
    switch (cmd->options[i].flag) {
    case '\0':
      break;
    case 'b':
      if (got_curve_opt) {
        sprintf(p->parser.error_msg_buffer,
                "cannot pass -b (decibels) after -e (curve)");
        return 10;
      }
      values_in_db = 1;
      break;
    case 'e':
      got_curve_opt = 1;
      curve_pow
          = rt_float_from_any_numeric_token(&(cmd->options[i].opt_args[0]));
      if (curve_pow > 10.f || curve_pow < -10.f) {
        sprintf(p->parser.error_msg_buffer, "curve power %.2f is out of range",
                curve_pow);
        return 17;
      }

      value0 = rt_float_from_any_numeric_token(&cmd->options[i].opt_args[1]);
      if (values_in_db) {
        value0 = rt_dbtoa(value0);
        break;
      }
      break;
    default:
      break;
    }
  }

  /* get the command arg */
  rt_real val = rt_float_from_any_numeric_token(&cmd->command_args[0]);
  rt_uint status;
  if (values_in_db) {
    val = rt_dbtoa(val);
  }

  if (got_curve_opt) {
    valueN = val;
    status = rt_parser_exec_check_level(p, valueN);
    if (status)
      return status;
  }
  else {
    value0 = val;
  }
  status = rt_parser_exec_check_level(p, value0);
  if (status)
    return status;

  bin0 = cmd->command_args[1].raw_arg.irange[0];
  binN = cmd->command_args[1].raw_arg.irange[1];

  /* begin range checks */
  if (bin0 > rt_manip_len(p) || binN > rt_manip_len(p) || binN < bin0) {
    /* bin0 < 0 for uint is just a big number */
    sprintf(p->parser.error_msg_buffer, "Improper bin range: %i-%i", bin0,
            binN);
    return 11;
  }

  /* only mono for now */
  if (curve_pow == -100.f) {
    rt_manip_set_bins(p, p->chans[0], def->manip_flavor,
                      bin0, binN, value0);
  }
  else {
    rt_manip_set_bins_curved(p, p->chans[0], def->manip_flavor, bin0, binN,
                             value0, valueN, curve_pow);
  }
  sprintf(p->parser.error_msg_buffer, "successful parse of a %s command",
          cmd->name);
  rt_listener_return_t ret = rt_get_empty_listener_data();
  ret.manip_flavor         = def->manip_flavor;
  p->listener.listener_callback(p->listener.listener_obj, ret);
  return 0;
}
