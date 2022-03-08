#include "../rt_globals.h" /* for intellisense */
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
              "Unexpected error while searching for command by name.\n");
      return NULL;
    }
  }
  return NULL;
}
