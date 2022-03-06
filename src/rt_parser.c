#include "rtstft.h"

void rt_parser_lex_one(rt_arg *arg) {}
int  rt_parser_lex_string(rt_parser parser, const char *arg_str)
{
  if (parser->buffer_active) {
    fprintf(stderr, "Tried to double-fill parser buffer.\n");
    exit(-1);
  }
  rt_parser_clear_buffer(parser);
  parser->buffer_active = 1;

  rt_uint pos = 0, read_pos = 0, argc = 0, row_offset = 0;
  char    in_token;
  do {
    in_token = 1;
    do {
      switch (arg_str[read_pos++]) {
      case '-':
        if (isdigit(arg_str)) {
          do {

          } while (arg_str[read_pos]);
          in_token = 0;
        }
        if (in_token) {
          while (isalpha(arg_str[read_pos++])) {
          }
        }
        if (0) {
          fprintf(stderr, "Unexpected char %c during lexing.\n",
                  arg_str[read_pos]);
          return 1;
        }
      case ' ':
        break;
      default:
        fprintf(stderr, "Unexpected error during lexing.\n");
        return 1;
        break;
      }

    } while (in_token);
    pos = 0;
    row_offset += RT_CMD_MAX_ARG_LEN;
  } while (++argc < RT_CMD_MAX_ARGS);
  fprintf(stderr, "Arg string overflow.\n");
  return -2;
}

void rt_parser_clear_buffer(rt_parser parser)
{
  /* also sets buffer_active to 0 */
  memset(parser, 0, sizeof(rt_parser_t));
}

void rt_parser_parse_contents(rt_parser parser)
{
  strcpy(parser->current_cmd, parser->arg_opt_buffer[0]);
  char    was_arg_opt = 0;
  rt_uint i           = 1;
}

void rt_parse_and_execute(rt_params p, const char *arg_str)
{
  int status = rt_parser_lex_string(&(p->parser), arg_str);
  if (status) {
    fprintf(stderr, "Error occured during parsing.\n");
    exit(status);
  }
  rt_parser_parse_contents(&p->parser);
  rt_parser_clear_buffer(&p->parser);
}