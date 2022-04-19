#include "../rtstft.h"

rt_token_t *rt_parser_next_available_token_slot(rt_parser parser)
{
  rt_token_t *ptr = parser->token_buffer;
  do {
    if (ptr->token_flavor == RT_CMD_UNDEFINED_T) {
      return ptr;
    }
  } while (++ptr != parser->token_buffer + RT_CMD_ARGC_MAX);
  sprintf(parser->error_msg_buffer, "Ran out of token slots.\n");
  return NULL;
}

int rt_parser_lex_command(rt_parser parser, rt_token_t *token,
                          const char *raw_token)
{
  const rt_command_define_t *cmd
      = rt_parser_check_command_exists(parser, raw_token);
  if (cmd == NULL) {
    return 1;
  }
  token->token_flavor = RT_CMD_COMMAND_T;
  strcpy(token->raw_arg.str, raw_token);
  parser->active_cmd_def = cmd;
  return 0;
}

int rt_parser_lex_param(rt_parser parser, const char *current_token)
{
  rt_uint     i = 1;
  rt_token_t *token;
  do {
    token = rt_parser_next_available_token_slot(parser);
    if (token == NULL) {
      return 10;
    }
    token->token_flavor   = RT_CMD_PARAM_T;
    token->raw_arg.str[0] = current_token[i];
  } while (current_token[++i] != '\0');
  return 0;
}

int rt_parser_lex_numeric(rt_parser parser, rt_token_t *token,
                          const char *token_raw)
{
  char    check;
  rt_uint str_pos = 0, second_num_pos = 0, num_dashes = 0;
  do {
    check = token_raw[str_pos];
    if (!isdigit(check)) {
      switch (check) {
      case '.':
        if (token->token_flavor == RT_CMD_CHAN_RANGE_T) {
          sprintf(parser->error_msg_buffer, "channel range must be int.");
          return 25;
        }
        token->token_flavor = RT_CMD_FLOAT_T;
        break;
      case '-':
        if (num_dashes == 0) {
          if (str_pos == 0) {
            num_dashes = 1;
            break;
          }
          else {
            token->token_flavor = RT_CMD_CHAN_RANGE_T;
            second_num_pos      = str_pos + 1;
            num_dashes          = 1;
            break;
          }
        }
        sprintf(parser->error_msg_buffer, "too many dashes in numeric arg.");
        return 1;
      default:
        sprintf(parser->error_msg_buffer,
                "Unexpected char %c during numeric lexing\n", token_raw[1]);
        return 1;
      }
    }

  } while ((token_raw[++str_pos]) != '\0');
  if (token->token_flavor == RT_CMD_CHAN_RANGE_T) {
    token->raw_arg.irange[0] = atoi(token_raw);
    token->raw_arg.irange[1] = atoi(token_raw + second_num_pos);
    return 0;
  }
  else if (token->token_flavor == RT_CMD_FLOAT_T) {
    token->raw_arg.f = (rt_real)atof(token_raw);
    return 0;
  }
  else {
    char *end_ptr    = NULL;
    token->raw_arg.i = (rt_uint)strtol(token_raw, &end_ptr, 10);
    if (end_ptr == NULL) {
      /* don't crash the program */
    }
    else if (*end_ptr == '\0') {
      token->token_flavor = RT_CMD_INT_T;
      return 0;
    }
  }
  sprintf(parser->error_msg_buffer,
          "Unexpected error during numeric lexing.\n");
  return 1;
}

int rt_parser_lex_args(rt_parser parser)
{
  rt_uint     argc = 0;
  const char *token_raw;
  int         status;
  rt_token_t *token;
  do {
    token_raw = parser->argv[argc];
    token     = rt_parser_next_available_token_slot(parser);
    if (token == NULL) {
      return 10;
    }
    if (token_raw[0] == '-') {
      if (isalpha(token_raw[1])) {
        status = rt_parser_lex_param(parser, token_raw);
      }
      else if (isdigit(token_raw[1])) {
        status = rt_parser_lex_numeric(parser, token, token_raw);
      }
      else {
        sprintf(parser->error_msg_buffer, "Unexpected char %c during lexing\n",
                token_raw[1]);
        return 1;
      }
    }
    else if (isdigit(token_raw[0])) {
      status = rt_parser_lex_numeric(parser, token, token_raw);
    }
    else if (argc == 0) {
      status = rt_parser_lex_command(parser, token, token_raw);
    }
    else {
      sprintf(parser->error_msg_buffer, "Unexpected token %s during lexing\n",
              token_raw);
      return 1;
    }
    if (status != 0) {
      return status;
    }
  } while (parser->argv[++argc][0] != '\0');
  if (argc >= RT_CMD_ARGC_MAX || argc == 0) {
    return 1;
  }
  return 0;
}
