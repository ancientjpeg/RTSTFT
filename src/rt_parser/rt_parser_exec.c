#include "../rtstft.h"
int rt_parser_execute_gain_gate_limit(rt_params p);
int rt_parser_execute_gain(void *params_ptr)
{
  rt_params p = (rt_params)params_ptr;
  return 1;
}
int rt_parser_execute_gate(void *params_ptr)
{
  rt_params p = (rt_params)params_ptr;
  return 1;
}
int rt_parser_execute_limit(void *params_ptr)
{
  rt_params p = (rt_params)params_ptr;
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
  rt_manip_flavor_t m_flav = p->parser.active_cmd_def->manip_flavor;

  return 1;
}
