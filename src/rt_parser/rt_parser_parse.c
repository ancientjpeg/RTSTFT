#include "../rtstft.h"

const rt_command_define_t *rt_parser_check_command_exists(rt_parser   parser,
                                                          const char *token)
{
  rt_uint search_pos = RT_CMD_ALL_COMMANDS_COUNT / 2, str_pos = 0, temp;
  char    token_char, cmd_char;
  /* because unsigned, also tests for below zero case */
  while (search_pos < RT_CMD_ALL_COMMANDS_COUNT) {
    cmd_char   = cmd_table[search_pos].name[str_pos];
    token_char = token[str_pos];
    if (cmd_char == '\0' && token_char == '\0') {
      return cmd_table + search_pos;
    }
    else if (cmd_char == token_char) {
      str_pos++;
    }
    else if (token_char < cmd_char) {
      search_pos /= 2;
      str_pos = 0;
    }
    else if (token_char > cmd_char) {
      temp       = search_pos / 2;
      temp       = temp == 0 ? 1 : temp;
      search_pos = search_pos + temp;
      str_pos    = 0;
    }
    else {
      sprintf(parser->error_msg_buffer,
              "Unexpected error while searching for command by name.");
      return NULL;
    }
  }
  return NULL;
}

int rt_parser_parse_opt(rt_parser parser, rt_uint *const token_index)
{
  char target_flag     = parser->token_buffer[*token_index].raw_arg.str[0];
  int  parsed_opt_args = 0, i = 0;
  const rt_option_define_t *opt_def = NULL, *opd_temp;
  do {
    opd_temp = parser->active_cmd_def->opts + i;
    if (parser->token_buffer[*token_index].raw_arg.str[0] == opd_temp->flag) {
      opt_def = opd_temp;
      break;
    }
  } while (++i < parser->active_cmd_def->optc);

  /* after loop, check that we found the flag */
  if (opt_def == NULL) {
    sprintf(parser->error_msg_buffer,
            "Could not find provided option %c for command %s.", target_flag,
            parser->active_cmd_def->name);
    return 1;
  }

  /* find next writeable option slot in the command object */
  rt_opt_t *writeable_opt = NULL, *opt_temp;
  i                       = 0;
  while (i < RT_CMD_OPT_ARGC_MAX) {
    opt_temp = parser->command.options + i++;
    if (opt_temp->flag == '\0') {
      writeable_opt       = opt_temp;
      writeable_opt->flag = opt_def->flag;
      break;
    }
  }
  if (writeable_opt == NULL) {
    sprintf(parser->error_msg_buffer,
            "Tried to use too many options with command %s. last flag: %c.",
            parser->active_cmd_def->name, opt_def->flag);
    return 1;
  }
  rt_token_t *token_curr;
  if (opt_def->argc == 0) {
    ++*token_index;
    return 0;
  }
  while (parsed_opt_args < opt_def->argc) {
    token_curr = parser->token_buffer + ++*token_index;
    if (token_curr->token_flavor == RT_CMD_PARAM_T
        || token_curr->token_flavor == RT_CMD_UNDEFINED_T) {
      sprintf(parser->error_msg_buffer,
              "Did not recieve enough args during opt-arg parsing.");
      return 1;
    }
    writeable_opt->opt_args[parsed_opt_args++] = *token_curr;
    if (parsed_opt_args == RT_CMD_OPT_ARGC_MAX) {
      ++*token_index;
      return 0;
    }
  }
  sprintf(parser->error_msg_buffer, "Unexpected error during opt-arg parsing.");
  return 1;
}

int rt_parser_parse_in_place(rt_parser parser)
{
  const rt_command_t *cmd = &(parser->command);
  strcpy((char *)cmd->name, (char *)parser->active_cmd_def->name);
  rt_uint           token_index = 1;
  int               status = 0, parsed_cmd_args = 0;
  const rt_token_t *token;
  while ((token = parser->token_buffer + token_index)->token_flavor
         != RT_CMD_UNDEFINED_T) {
    if (token->token_flavor == RT_CMD_PARAM_T) {
      if (parsed_cmd_args) {
        sprintf(parser->error_msg_buffer,
                "Encountered an option flag while parsing command args.");
        return 15;
      }
      status = rt_parser_parse_opt(parser, &token_index);
      if (status) {
        return status;
      }
    }
    else {
      parser->command.command_args[parsed_cmd_args++]
          = parser->token_buffer[token_index++];
    }
  }
  return 0;
}