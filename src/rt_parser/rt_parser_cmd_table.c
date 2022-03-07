#include "rt_parser.h"

const rt_command_table_t cmd_table = (rt_command_table_t){
    {{"gain",
      {RT_CMD_CHAN_RANGE_T, RT_CMD_FLOAT_T},
      {{'c', {RT_CMD_FLOAT_T, RT_CMD_FLOAT_T, RT_CMD_FLOAT_T}}}},
     {"gate",
      {RT_CMD_CHAN_RANGE_T, RT_CMD_FLOAT_T},
      {{'c', {RT_CMD_FLOAT_T, RT_CMD_FLOAT_T, RT_CMD_FLOAT_T}}}},
     {"limit",
      {RT_CMD_CHAN_RANGE_T, RT_CMD_FLOAT_T},
      {{'c', {RT_CMD_FLOAT_T, RT_CMD_FLOAT_T, RT_CMD_FLOAT_T}}}},
     {"rebase", {}, {}},
     {"transpose",
      {RT_CMD_INT_T},
      {{'c', {RT_CMD_FLOAT_T}}, {'s', {RT_CMD_FLOAT_T}}, {}}}}};