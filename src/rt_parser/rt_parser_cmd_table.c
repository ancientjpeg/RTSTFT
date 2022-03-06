#include "rt_parser.h"
const rt_command_table_t cmd_table = (rt_command_table_t){
    {{"gain", {RT_INT_RANGE_ARG, RT_FLOAT_ARG}, {}},
     {"limit",
      {RT_INT_RANGE_ARG, RT_FLOAT_ARG},
      {{'c', {RT_FLOAT_ARG, RT_FLOAT_ARG, RT_FLOAT_ARG}}}},
     {"transpose",
      {RT_INT_ARG},
      {{'c', {RT_FLOAT_ARG}}, {'s', {RT_FLOAT_ARG}}}}}};