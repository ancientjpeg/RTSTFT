#ifndef RT_PARSER_H
#define RT_PARSER_H

#include "../rt_globals.h"

#define RT_CMD_ARGC_MAX 10
#define RT_CMD_ARG_LEN_MAX 10
#define RT_CMD_BUFFER_LEN (RT_CMD_ARGC_MAX * RT_CMD_ARG_LEN_MAX)
#define RT_CMD_NAME_LEN 10
#define RT_CMD_MAX_OPTS 4
#define RT_CMD_OPT_ARGC_MAX 3
#define RT_CMD_COMMAND_ARGC_MAX 3
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

/* ========    structure for command table    ======== */
typedef struct RTSTFT_Command_Table {
  const struct RTSTFT_Command_Define {
    const char        name[RT_CMD_NAME_LEN];
    const rt_arg_type cmd_argtypes[RT_CMD_COMMAND_ARGC_MAX];
    const struct RTSTFT_Option_Define {
      const char        flag;
      const rt_arg_type opt_argtypes[RT_CMD_OPT_ARGC_MAX];
    } opts[RT_CMD_MAX_OPTS];
  } commands[RT_CMD_ALL_COMMANDS_COUNT];
} rt_command_table_t;

/* ========     structure for lexed args      ======== */
typedef struct RTSTFT_Lexed_Arg {
  union {
    char    str[RT_CMD_ARG_LEN_MAX];
    rt_real f;
    int     i;
    int     irange[2];
  } raw_arg;
  rt_arg_type arg_type;
} rt_arg_t;

/* ========      structs for parsed cmds      ======== */
typedef struct RTSTFT_Parsed_Option {
} rt_opt_t;
typedef struct RTSTFT_Parsed_Command {
  char     name[RT_CMD_NAME_LEN];
  rt_arg_t command_args[RT_CMD_COMMAND_ARGC_MAX];
  rt_opt_t options[RT_CMD_MAX_OPTS];
} rt_command_t;

typedef struct RTSTFT_Parser {
  rt_arg_t token_buffer[RT_CMD_ARGC_MAX];
  char     buffer_active;
} rt_parser_t;
typedef rt_parser_t            *rt_parser;

const extern rt_command_table_t cmd_table;

/* ========            functions             ======== */
void rt_parser_clear_buffer(rt_parser parser);

#endif