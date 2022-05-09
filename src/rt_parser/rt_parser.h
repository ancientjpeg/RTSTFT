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

#define RT_CMD_ALL_COMMANDS_COUNT (RU(7))
#define RT_CMD_MAX_SEARCH_DEPTH (rt_log2_floor(RT_CMD_ALL_COMMANDS_COUNT))

typedef enum RT_TOKEN_FLAVORS {
  RT_CMD_UNDEFINED_T,
  RT_CMD_PARAM_T,
  RT_CMD_INT_T,
  RT_CMD_CHAN_RANGE_T,
  RT_CMD_FLOAT_T,
  RT_CMD_STRING_T,
  RT_CMD_COMMAND_T,
  RT_CMD_NUM_TOKEN_FLAVORS
} rt_token_flavor;

/* ========    structure for command table    ======== */

typedef struct RTSTFT_Option_Define {
  const char            flag;
  const int             argc;
  const rt_token_flavor opt_argtypes[RT_CMD_OPT_ARGC_MAX];
} rt_option_define_t;
typedef struct RTSTFT_Command_Define {
  const char        name[RT_CMD_NAME_LEN];
  rt_manip_flavor_t manip_flavor;
  int (*exec_func)(void *);
  const int                argc;
  const rt_token_flavor    cmd_argtypes[RT_CMD_COMMAND_ARGC_MAX];
  const int                optc;
  const rt_option_define_t opts[RT_CMD_MAX_OPTS];
} rt_command_define_t;

/* ========    structure for lexed tokens     ======== */
typedef struct RTSTFT_Lexed_Token {
  union {
    char    str[RT_CMD_ARG_LEN_MAX];
    rt_real f;
    int     i;
    int     irange[2];
  } raw_arg;
  rt_token_flavor token_flavor;
} rt_token_t;

/* ========      structs for parsed cmds      ======== */
typedef struct RTSTFT_Parsed_Option {
  char       flag;
  rt_token_t opt_args[RT_CMD_OPT_ARGC_MAX];
} rt_opt_t;
typedef struct RTSTFT_Parsed_Command {
  char       name[RT_CMD_NAME_LEN];
  rt_token_t command_args[RT_CMD_COMMAND_ARGC_MAX];
  rt_opt_t   options[RT_CMD_MAX_OPTS];
} rt_command_t;

typedef struct RTSTFT_Parser {
  char                       argv[RT_CMD_ARGC_MAX][RT_CMD_ARG_LEN_MAX];
  rt_token_t                 token_buffer[RT_CMD_ARGC_MAX];
  const rt_command_define_t *active_cmd_def;
  rt_command_t               command;
  char                       error_msg_buffer[100];
  char                       buffer_active;
} rt_parser_t;
typedef rt_parser_t             *rt_parser;

const extern rt_command_define_t cmd_table[];

/* ========            functions             ======== */
void                       rt_parser_clear_buffer(rt_parser parser);
const rt_command_define_t *rt_parser_check_command_exists(rt_parser   parser,
                                                          const char *token);
int                        rt_parser_lex_args(rt_parser parser);
int                        rt_parser_parse_in_place(rt_parser parser);

/* ========      command exec functions      ======== */
int rt_parser_execute_flush(void *params_ptr);
int rt_parser_execute_gain(void *params_ptr);
int rt_parser_execute_gate(void *params_ptr);
int rt_parser_execute_limit(void *params_ptr);
int rt_parser_execute_rebase(void *params_ptr);
int rt_parser_execute_reset(void *params_ptr);
int rt_parser_execute_transpose(void *params_ptr);

#endif
