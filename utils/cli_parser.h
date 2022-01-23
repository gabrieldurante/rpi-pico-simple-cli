/*******************************************************************************
 *   @file   cli_parser.h
 *   @brief  Header file of cli parser implementation
 *   @author gdurante (gbdurante@gmail.com)
 ********************************************************************************/

#ifndef __CLI_PARSER_H
#define __CLI_PARSER_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

struct cli_parser_cmd_option;
struct cli_parser_parsed_arg;

typedef void (*command_callback_t)(int argc,
                                   const struct cli_parser_parsed_arg *args);

typedef union {
  char *argument_s;
  int argument_i;
  float argument_f;
} cli_parsed_arg_t;

struct cli_parser_cmd_option {
  const char *name;
  const char *help_msg;
  const char *optstring;
  const char *optypes;
  int argc;
  command_callback_t callback;
};

struct cli_parser_parsed_arg {
  char type;
  cli_parsed_arg_t value;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

void cli_parser_init(void);
void cli_parser_proc(void);
void cli_parser_register_commands(const struct cli_parser_cmd_option *opts);

#endif /* __CLI_PARSER_H */
