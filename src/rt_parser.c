#include "rtstft.h"

int rt_parser_get_argv(rt_parser parser, const char *arg_str)
{
  if (parser->buffer_active) {
    fprintf(stderr, "Tried to double-fill parser buffer.\n");
    exit(-1);
  }
  rt_parser_clear_buffer(parser);
  parser->buffer_active = 1;

  rt_uint pos = 0, str_pos = 0, arg_curr = 0, row_offset = 0;
  char    curr;
  do {
    do {
      curr = arg_str[str_pos++];
      if (curr == ' ') {
        if (pos == 0) {
          fprintf(stderr, "Too many spaces in arg string.\n");
          return -1;
        }
        parser->arg_buffer[arg_curr][pos] = '\0';
        break;
      }
      else if (curr == '\0') {
        parser->arg_buffer[arg_curr][pos] = '\0';
        return 0;
      }
      parser->arg_buffer[arg_curr][pos] = curr;
    } while (++pos < RT_SCRIPT_MAX_ARG_LENGTH);
    pos = 0;
    row_offset += RT_SCRIPT_MAX_ARG_LENGTH;
  } while (++arg_curr < RT_SCRIPT_MAX_ARGS);
  fprintf(stderr, "Arg string overflow.\n");
  return -2;
}

void rt_parser_clear_buffer(rt_parser parser)
{
  rt_uint buf_size = RT_SCRIPT_BUFFER_LENGTH;
  memset(parser->arg_buffer, 0, buf_size);
  parser->buffer_active = 0;
}

void rt_parse_and_execute(rt_params p, const char *arg_str)
{
  rt_parser_get_argv(&(p->parser), arg_str);
  rt_parser_clear_buffer(&(p->parser));
}