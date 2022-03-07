#include "../rtstft.h"

int rt_parser_lex_numeric(rt_lexed_arg_t *arg)
{
  rt_uint num_start = 0;
  int     sign      = 1;
  char   *dash_ptr  = strchr(arg->raw_arg.str, '-');
  if (dash_ptr == arg->raw_arg.str) {
    sign = -1;
    if (strchr(arg->raw_arg.str + 1, '-') != NULL) {
      fprintf(stderr, "negative numbers in int range not allowed.\n");
      return -10;
    }
    num_start = 1;
  }
  else if (dash_ptr != NULL) {
    arg->arg_type           = RT_INT_RANGE_ARG;
    int second_num_position = dash_ptr - (char *)arg->raw_arg.str + 1;
    int i0                  = atoi(arg->raw_arg.str);
    int i1                  = atoi(arg->raw_arg.str + second_num_position);
    arg->raw_arg.irange[0]  = i0;
    arg->raw_arg.irange[1]  = i1;
    return 0;
  }

  if (strchr(arg->raw_arg.str, '.') != NULL) {
    arg->raw_arg.f = (rt_real)sign * atof(arg->raw_arg.str + num_start);
    arg->arg_type  = RT_FLOAT_ARG;
    return 0;
  }
  char *check;
  int   temp = sign * strtol(arg->raw_arg.str + num_start, &check, 10);
  if (*check != '\0') {
    fprintf(stderr, "unexpected error during numeric lexing.\n");
    return 1;
  }
  arg->raw_arg.i = temp;
  arg->arg_type  = RT_INT_ARG;
  return 0;
}

int rt_parser_lex_one(rt_lexed_arg_t *arg)
{
  switch (arg->arg_type) {
  case RT_COMMAND_ARG:
    /* maybe check command existence here? */
    return 0;
  case RT_PARAM_ARG:
    if (arg->raw_arg.str[1] != '\0' || !isalpha(arg->raw_arg.str[0])) {
      fprintf(stderr, "invalid param token.\n");
      return -5;
    }
    return 0;
  default:
    if (isdigit(arg->raw_arg.str[0]) || arg->raw_arg.str[0] == '-') {
      int status = rt_parser_lex_numeric(arg);
      if (status) {
        return status;
      }
      return 0;
    }
    if (arg->raw_arg.str[0] == '\0') {
      return 42;
    }
    fprintf(stderr, "unexpected char %c (%d) during token lexing.\n",
            arg->raw_arg.str[0], arg->raw_arg.str[0]);
    return 1;
  }
}

int rt_parser_lex_args(rt_parser parser)
{
  int     status = 1;
  rt_uint i;
  for (i = 0; i < RT_CMD_ARGC_MAX; i++) {
    status = rt_parser_lex_one(parser->token_buffer + i);
    if (status == 42) {
      return 0;
    }
    else if (status != 0) {
      return status;
    }
  }
  return 1;
}
int rt_parser_split_argv(rt_parser parser, const char *arg_str)
{
  if (parser->buffer_active) {
    fprintf(stderr, "Tried to double-fill parser buffer.\n");
    exit(-1);
  }
  rt_parser_clear_buffer(parser);
  parser->buffer_active = 1;

  int     status        = 0;
  char    curr;
  rt_uint pos = 0, read_pos = 0, argc = 0, row_offset = 0;
  while (arg_str[read_pos] != ' ') {
    parser->token_buffer[argc].raw_arg.str[pos++] = arg_str[read_pos++];
  }
  parser->token_buffer[argc].arg_type = RT_COMMAND_ARG;
  read_pos++;
  pos  = 0;
  argc = 1;

  do {
    switch (arg_str[read_pos]) {
    case '-':
      if (isalpha(arg_str[read_pos + 1])) {
        while (isalpha(arg_str[++read_pos])) {
          parser->token_buffer[argc].arg_type         = RT_PARAM_ARG;
          parser->token_buffer[argc].raw_arg.str[0]   = arg_str[read_pos];
          parser->token_buffer[argc++].raw_arg.str[1] = '\0';
        }
        if (strchr(" \0", arg_str[read_pos]) == NULL) {
          fprintf(stderr, "unexpected char %c (%d) during argv separation.\n",
                  arg_str[read_pos], arg_str[read_pos]);
          return 1;
        }
        read_pos++;
      }
    default:
      while (arg_str[read_pos] != ' ') {
        curr = arg_str[read_pos++];
        if (curr == '\0') {
          return status;
        }
        parser->token_buffer[argc].raw_arg.str[pos++] = curr;
      }
      parser->token_buffer[argc].raw_arg.str[pos] = '\0';
      read_pos++;
      break;
    }

    pos = 0;
    row_offset += RT_CMD_ARG_LEN_MAX;
  } while (++argc < RT_CMD_ARGC_MAX);
  fprintf(stderr, "Arg string overflow.\n");
  return -2;
}

void rt_parser_clear_buffer(rt_parser parser)
{
  /* also sets buffer_active to 0 */
  memset(parser, 0, sizeof(rt_parser_t));
}

void rt_parse_and_execute(rt_params p, const char *arg_str)
{
  int status = rt_parser_split_argv(&p->parser, arg_str);
  if (status) {
    fprintf(stderr, "Error occured during argv split.\n");
    exit(status);
  }
  status = rt_parser_lex_args(&p->parser);
  if (status) {
    fprintf(stderr, "Error occured during lexing.\n");
    exit(status);
  }

  rt_parser_clear_buffer(&p->parser);
}