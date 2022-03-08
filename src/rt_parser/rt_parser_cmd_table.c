#include "rt_parser.h"

const rt_command_define_t cmd_table[RT_CMD_ALL_COMMANDS_COUNT]
    = {{"gain",
        2,
        {RT_CMD_CHAN_RANGE_T, RT_CMD_FLOAT_T},
        {{'c', 3, {RT_CMD_FLOAT_T, RT_CMD_FLOAT_T, RT_CMD_FLOAT_T}}}},
       {"gate",
        2,
        {RT_CMD_CHAN_RANGE_T, RT_CMD_FLOAT_T},
        {{'c', 3, {RT_CMD_FLOAT_T, RT_CMD_FLOAT_T, RT_CMD_FLOAT_T}}}},
       {"limit",
        2,
        {RT_CMD_CHAN_RANGE_T, RT_CMD_FLOAT_T},
        {{'c', 3, {RT_CMD_FLOAT_T, RT_CMD_FLOAT_T, RT_CMD_FLOAT_T}}}},
       {"rebase", {}, {}},
       {"transpose",
        1,
        {RT_CMD_INT_T},
        {{'c', 1, {RT_CMD_FLOAT_T}}, {'s', 1, {RT_CMD_FLOAT_T}}, {}}}};