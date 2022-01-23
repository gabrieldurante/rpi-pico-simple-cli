/*******************************************************************************
 *   @file   cli_parser.c
 *   @brief  Source file of cli parser implementation
 *   @author gdurante (gbdurante@gmail.com)
 ********************************************************************************/

#include "pico/stdlib.h"

#include "cli_parser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
// #define DEBUG

#define CLI_PARSER_ECHO_BACK
#define CLI_RX_BUFFERSIZE 1024
#define CLI_TOKENS_MAX 32
#define CLI_TOKEN_MATCH(X, Y) strcmp(X, (char const *)Y) == 0
#define CLI_COMMANDS_MAX 32

#define ASCII_EOT 4
#define ASCII_ENQ 5
#define ASCII_BS 8
#define ASCII_TAB 9
#define ASCII_LF 10
#define ASCII_VT 11
#define ASCII_FF 12
#define ASCII_CR 13
#define ASCII_SPC 32
#define ASCII_DEL 127
#define ASCII_NOT_CTRL(X)                                                      \
  ((X != ASCII_EOT && X != ASCII_ENQ && X != ASCII_LF && X != ASCII_FF &&      \
    X != ASCII_CR && X != ASCII_DEL && X != ASCII_BS))
#define ASCII_ISSPACE(X)                                                       \
  ((X == ASCII_SPC || X == ASCII_TAB || X == ASCII_LF || X == ASCII_VT ||      \
    X == ASCII_FF || X == ASCII_CR))
#define ASCII_IS_FMT_CHAR(X)                                                   \
  ((X == 'i' || X == 'd' || X == 'o' || X == 'x' || X == 'f' || X == 'c' ||    \
    X == 's'))

/******************************************************************************/
/*************************** Local Variables **********************************/
/******************************************************************************/

static uint8_t cli_rx_buffer[CLI_RX_BUFFERSIZE] = {0};
static uint8_t cli_rx_buffer_idx = 0;

static struct cli_parser_cmd_option cmd_option_list[CLI_COMMANDS_MAX] = {NULL};

/******************************************************************************/
/********************* Local Functions Definitions ****************************/
/******************************************************************************/

static void help_callback(int argc, const struct cli_parser_parsed_arg *args) {
  for (int i = 0; i < CLI_COMMANDS_MAX; i++) {
    if (cmd_option_list[i].name != NULL) {
      printf("[%s] [%i] [%s] [%s] [%s]\n", cmd_option_list[i].name,
             cmd_option_list[i].argc,
             (cmd_option_list[i].optstring != NULL)
                 ? cmd_option_list[i].optstring
                 : "noargs",
             (cmd_option_list[i].optypes != NULL) ? cmd_option_list[i].optypes
                                                  : "notypes",
             cmd_option_list[i].help_msg);
    }
  }
}

static void cli_clear_buffer(void) {
  memset(&cli_rx_buffer[0], 0, sizeof(cli_rx_buffer));
  cli_rx_buffer_idx = 0;
}

static uint16_t cli_tokenize_args(uint8_t *str, uint8_t **tokens) {
  if (!str) {
    return 0;
  }

  uint8_t nargs = 0;

  // catch 1st arg
  if (!ASCII_ISSPACE(*str)) {
    tokens[nargs++] = str;
  }

  // run through the args
  while (*str != '\0') {
    char *next_char = (char *)(str + 1);
    if (ASCII_ISSPACE(*str) && !ASCII_ISSPACE(*next_char)) {
      tokens[nargs++] = (uint8_t *)next_char;
      *str = '\0';
    }
    str++;
  }

#if defined(DEBUG)
  printf("nargs[%i]\n", nargs);
#endif
  return nargs;
}

static uint16_t cli_tokenize_options(uint8_t *str, uint8_t **tokens) {
  if (!str) {
    return 0;
  }

  uint8_t nopts = 0;

  // run through the format string
  while (*str != '\0') {
    char delimiter = *(str - 1);
    if (ASCII_IS_FMT_CHAR(*str) && delimiter == '%') {
#if defined(DEBUG)
      printf("fmt[%c%c]\n", delimiter, *str);
#endif
      tokens[nopts++] = str;
    }
    str++;
  }

#if defined(DEBUG)
  printf("nopts[%i]\n", nopts);
#endif
  return nopts;
}

static char cli_parse_option_type(int opt, const char *optypes, int argc,
                                  int optind) {
  char *opts[CLI_TOKENS_MAX] = {0};
  uint16_t optc = cli_tokenize_options((uint8_t *)optypes, (uint8_t **)opts);

  if (optc == argc) {
    char opt = *opts[optind];
#if defined(DEBUG)
    printf("opt[%c] optind[%i]\n", opt, optind);
#endif
    return opt;
  }

  return -1;
}

static void cli_arg_parse(uint8_t *str) {
  char *argv[CLI_TOKENS_MAX] = {0};
  uint16_t argc = cli_tokenize_args(str, (uint8_t **)argv);

  // process command
  for (int i = 0; i < CLI_COMMANDS_MAX; i++) {
    if (CLI_TOKEN_MATCH(argv[0], cmd_option_list[i].name)) {
      struct cli_parser_cmd_option cmd_option = cmd_option_list[i];
      // parse args
      if (argc > 1 && argc == (cmd_option.argc + 1)) {
        optind = 0;
        int parsed_arg_idx = 0;
        struct cli_parser_parsed_arg cmd_parsed_arg_list[CLI_TOKENS_MAX];
        while (1) {
#if defined(DEBUG)
          printf("getopt argc[%i] optstring[%s]\n", argc, cmd_option.optstring);
#endif
          int opt;
          opt = getopt(argc, argv, cmd_option.optstring);
          if (opt == -1 || opt == '?') {
#if defined(DEBUG)
            printf("getopt opt[%s]\n", opt == -1 ? "-1" : "?");
#endif
            break;
          }

#if defined(DEBUG)
          printf("opttypes [%s] idx[%i]\n", cmd_option.optypes, parsed_arg_idx);
#endif
          char opttype = cli_parse_option_type(opt, cmd_option.optypes,
                                               cmd_option.argc, parsed_arg_idx);
          if (opttype != -1) {
#if defined(DEBUG)
            printf("found matching opt[%c] optind[%i] type[%c] optarg[%s]\n",
                   opt, optind, opttype, optarg);
#endif
            // decode and store argument
            switch (opttype) {
            case 's':
              cmd_parsed_arg_list[parsed_arg_idx].type = opttype;
              cmd_parsed_arg_list[parsed_arg_idx].value.argument_s = optarg;
              parsed_arg_idx++;
              break;
            case 'i':
              cmd_parsed_arg_list[parsed_arg_idx].type = opttype;
              cmd_parsed_arg_list[parsed_arg_idx].value.argument_i =
                  atoi(optarg);
              parsed_arg_idx++;
              break;
            case 'f':
              cmd_parsed_arg_list[parsed_arg_idx].type = opttype;
              cmd_parsed_arg_list[parsed_arg_idx].value.argument_f =
                  atof(optarg);
              parsed_arg_idx++;
              break;
            default:
              break;
            }
          }
        }
        // callback
        if (cmd_option.callback) {
          cmd_option.callback(argc, cmd_parsed_arg_list);
        }
        return;
      } else if (argc == 1) {
        // callback
        if (cmd_option.callback) {
          cmd_option.callback(argc, NULL);
        }
        return;
      }
      printf("invalid number of arguments\n");
      return;
    }
  }
  printf("command %s not found\n", argv[0]);
}

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

void cli_parser_init(void) {
  // register help command
  cmd_option_list[0].name = "help";
  cmd_option_list[0].help_msg = "print this help";
  cmd_option_list[0].optstring = 0;
  cmd_option_list[0].optypes = 0;
  cmd_option_list[0].argc = 0;
  cmd_option_list[0].callback = help_callback;
}

void cli_parser_proc(void) {
  int rc = getchar_timeout_us(100);
  if (rc == PICO_ERROR_TIMEOUT) {
    return;
  }

  uint8_t cli_rx_data = (rc & 0xff);

  // check buffer size
  if (cli_rx_buffer_idx < CLI_RX_BUFFERSIZE) {
    // process current byte
    if (ASCII_NOT_CTRL(cli_rx_data)) // ignore unused ASCII codes
    {
      cli_rx_buffer[cli_rx_buffer_idx] = cli_rx_data;
      cli_rx_buffer_idx++;

#if defined(CLI_PARSER_ECHO_BACK)
      putchar_raw(cli_rx_data); // echo data back to terminal
#endif
    } else if (cli_rx_data == ASCII_BS ||
               cli_rx_data ==
                   ASCII_DEL) // detele current data if backspace or delete
    {
      cli_rx_buffer[cli_rx_buffer_idx] = 0;
      if (cli_rx_buffer_idx >= 1) {
        cli_rx_buffer_idx--;
      }

#if defined(CLI_PARSER_ECHO_BACK)
      putchar_raw(cli_rx_data); // echo data back to terminal
#endif
    } else if (cli_rx_data == ASCII_CR) {
      if (cli_rx_buffer_idx == 0) {
        return;
      }
      // valid command, process it
#if defined(CLI_PARSER_ECHO_BACK)
      putchar_raw(ASCII_FF); // clear terminal
#endif
      printf("cli> %s\n", &cli_rx_buffer[0]);
      cli_arg_parse(&cli_rx_buffer[0]);
      cli_clear_buffer();
    }
  } else {
    cli_clear_buffer();
  }
}

void cli_parser_register_commands(const struct cli_parser_cmd_option *opts) {
  for (int i = 0; opts[i].name != NULL; i++) {
    for (int j = 0; j < CLI_COMMANDS_MAX; j++) {
      if (cmd_option_list[j].name == NULL) {
        // allocate into command list
        cmd_option_list[j].name = opts[i].name;
        cmd_option_list[j].help_msg = opts[i].help_msg;
        cmd_option_list[j].optstring = opts[i].optstring;
        cmd_option_list[j].optypes = opts[i].optypes;
        cmd_option_list[j].argc = opts[i].argc;
        cmd_option_list[j].callback = opts[i].callback;
        break;
      }
    }
  }
}
