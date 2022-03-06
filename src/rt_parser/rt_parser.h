#ifndef RT_PARSER_H
#define RT_PARSER_H

#include "../rt_globals.h"

#define RT_CMD_ARGC_MAX 10
#define RT_CMD_ARG_LEN_MAX 10
#define RT_CMD_BUFFER_LEN (RT_CMD_ARGC_MAX * RT_CMD_ARG_LEN_MAX)
#define RT_CMD_NAME_LEN 10
#define RT_CMD_MAX_OPTS 4
#define RT_CMD_MAX_OPT_ARGS 3
#define RT_CMD_MAX_COMMAND_ARGS 3
#define RT_CMD_ALL_COMMANDS_COUNT 20

typedef enum RT_ARG_TYPES {
  RT_UNDEFINED_ARG,
  RT_PARAM_ARG,
  RT_INT_ARG,
  RT_INT_RANGE_ARG,
  RT_FLOAT_ARG,
  RT_STRING_ARG,
  RT_COMMAND_ARG,
  NUM_RT_ARG_TYPES
} rt_arg_type;
typedef struct RTSTFT_Command_Define {
  const char        name[RT_CMD_NAME_LEN];
  const rt_arg_type cmd_argtypes[RT_CMD_MAX_COMMAND_ARGS];
  const struct RTSTFT_Option_Define {
    const char        flag;
    const rt_arg_type opt_argtypes[RT_CMD_MAX_OPT_ARGS];
  } opts[RT_CMD_MAX_OPTS];
} rt_command_define_t;

typedef struct RTSTFT_Command_Table {
  const rt_command_define_t commands[RT_CMD_ALL_COMMANDS_COUNT];
} rt_command_table_t;

typedef struct RTSTFT_Lexed_Arg {
  union {
    char    str[RT_CMD_ARG_LEN_MAX];
    rt_real f;
    int     i;
    int     irange[2];
  } raw_arg;
  rt_arg_type arg_type;
} rt_arg;

typedef struct RTSTFT_Parser {
  rt_arg token_buffer[RT_CMD_ARGC_MAX];
  char   buffer_active;
} rt_parser_t;
typedef rt_parser_t            *rt_parser;

const extern rt_command_table_t cmd_table;

#endif