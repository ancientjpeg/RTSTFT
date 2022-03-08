#include "../rtstft.h"

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
    rt_parser_clear_buffer(&p->parser);
    return status;
  }
  status = rt_parser_lex_args(&p->parser);
  if (status) {
    rt_parser_clear_buffer(&p->parser);
    return status;
  }
  rt_parser_clear_buffer(&p->parser);
  return 0;
}