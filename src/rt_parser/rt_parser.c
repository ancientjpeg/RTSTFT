#include "../rtstft.h"

rt_token_t *rt_parser_next_available_token_slot(rt_parser parser)
{
  rt_token_t *ptr = parser->token_buffer;
  do {
    if (ptr->token_flavor == RT_CMD_UNDEFINED_T) {
      return ptr;
    }
  } while (++ptr != parser->token_buffer + RT_CMD_ARGC_MAX);
  return NULL;
}

int rt_parser_lex_numeric(rt_token_t *arg, char *error_msg_buffer)
{
  rt_uint num_start = 0;
  int     sign      = 1;
  char   *dash_ptr  = strchr(arg->raw_arg.str, '-');
  if (dash_ptr == arg->raw_arg.str) {
    sign = -1;
    if (strchr(arg->raw_arg.str + 1, '-') != NULL) {
      sprintf(error_msg_buffer, "negative numbers in int range not allowed.\n");
      return -10;
    }
    num_start = 1;
  }
  else if (dash_ptr != NULL) {
    arg->token_flavor       = RT_CMD_CHAN_RANGE_T;
    int second_num_position = dash_ptr - (char *)arg->raw_arg.str + 1;
    int i0                  = atoi(arg->raw_arg.str);
    int i1                  = atoi(arg->raw_arg.str + second_num_position);
    arg->raw_arg.irange[0]  = i0;
    arg->raw_arg.irange[1]  = i1;
    return 0;
  }

  if (strchr(arg->raw_arg.str, '.') != NULL) {
    arg->raw_arg.f    = (rt_real)sign * atof(arg->raw_arg.str + num_start);
    arg->token_flavor = RT_CMD_FLOAT_T;
    return 0;
  }
  char *check;
  int   temp = sign * strtol(arg->raw_arg.str + num_start, &check, 10);
  if (*check != '\0') {
    sprintf(error_msg_buffer, "unexpected error during numeric lexing.\n");
    return 1;
  }
  arg->raw_arg.i    = temp;
  arg->token_flavor = RT_CMD_INT_T;
  return 0;
}

int rt_parser_lex_param(rt_parser parser, const char *current_token)
{
  rt_uint     i = 1;
  rt_token_t *token;
  do {
    token = rt_parser_next_available_token_slot(parser);
    if (token == NULL) {
      sprintf(parser->error_msg_buffer,
              "Ran out of token slots during lexing.\n");
      return 10;
    }

    token->token_flavor   = RT_CMD_PARAM_T;
    token->raw_arg.str[0] = current_token[i];
  } while (current_token[++i] != '\0');
  return 0;
}

int rt_parser_lex_args(rt_parser parser)
{
  rt_uint     argc = 0;
  const char *this_arg;
  int         status;
  do {
    this_arg = parser->argv + argc;
    if (this_arg[0] == '-') {
      if (isalpha(this_arg[1])) {
        status = rt_parser_lex_param(parser, parser->argv[argc]);
      }
      else if (isdigit(this_arg[1])) {
      }
      else {
        sprintf(parser->error_msg_buffer, "Unexpected char %c during lexing\n",
                this_arg[1]);
      }
    }
  } while (parser->argv[++argc][0] != '\0');
  if (argc >= RT_CMD_ARGC_MAX || argc == 0) {
    return 1;
  }
  return 0;
}
int rt_parser_split_argv(rt_parser parser, const char *arg_str)
{
  if (parser->buffer_active) {
    sprintf(parser->error_msg_buffer, "Tried to double-fill parser buffer.\n");
    exit(-1);
  }
  rt_parser_clear_buffer(parser);
  parser->buffer_active = 1;

  char    curr, reading, end = 0;
  rt_uint pos = 0, read_pos = 0, argc = 0, row_offset = 0;

  do {
    reading = 1;
    do {
      curr = arg_str[read_pos++];
      switch (curr) {
      case '\0':
      case '\n':
        end = 1;
      case ' ':
        parser->argv[argc][pos] = '\0';
        if (end) {
          return 0;
        }
        reading = 0;
        break;
      default:
        parser->argv[argc][pos] = curr;
        break;
      }
    } while (++pos < RT_CMD_ARG_LEN_MAX && reading);
    if (reading) {
      sprintf(parser->error_msg_buffer, "Error: arg too long.\n");
      return -5;
    }
    pos = 0;
    row_offset += RT_CMD_ARG_LEN_MAX;
  } while (++argc < RT_CMD_ARGC_MAX);
  sprintf(parser->error_msg_buffer, "Arg string overflow.\n");
  return -2;
}

void rt_parser_clear_buffer(rt_parser parser)
{
  /* also sets buffer_active to 0 */
  memset(parser, 0, sizeof(rt_parser_t));
}

int rt_parse_and_execute(rt_params p, const char *arg_str)
{
  int status = rt_parser_split_argv(&p->parser, arg_str);
  if (status) {
    sprintf(p->parser.error_msg_buffer, "Error occured during argv split.\n");
    return status;
  }
  status = rt_parser_lex_args(&p->parser);
  if (status) {
    sprintf(p->parser.error_msg_buffer, "Error occured during lexing.\n");
    return status;
  }

  rt_parser_clear_buffer(&p->parser);
}