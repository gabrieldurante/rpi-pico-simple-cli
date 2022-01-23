#include "cli_parser.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include <stdio.h>

void cli_test_get_args(
    int argc, const struct cli_parser_parsed_arg *args) {
  if (args) {
    for (int i = 0; i < (argc - 1); i++) {
      switch (args[i].type) {
      case 's':
        printf("[%s] parsed arg [%s]\n", __FUNCTION__,
                args[i].value.argument_s);
        break;
      case 'i':
        printf("[%s] parsed arg [%i]\n", __FUNCTION__,
                args[i].value.argument_i);
        break;
      case 'f':
        printf("[%s] parsed arg [%f]\n", __FUNCTION__,
                args[i].value.argument_f);
        break;
      default:
        break;
      }
    }
  }
}

int main() {
  stdio_init_all();

  cli_parser_init();

  // clang-format off
  static struct cli_parser_cmd_option cli_options[] = {
      {"cli_test_get_single_arg", "get single arg from CLI", "t:", "%i", 1, cli_test_get_args},
      {"cli_test_get_double_args", "get double args from CLI", "t:v:", "%i%f", 2, cli_test_get_args},
      {"cli_test_get_triple_args", "get triple args from CLI", "t:v:j:", "%i%f%s", 3, cli_test_get_args},
      {0, 0, 0, 0, 0, 0} // sentinel
  };
  // clang-format on

  cli_parser_register_commands(cli_options);

  while (true) {
    cli_parser_proc();
    sleep_ms(1);
  }

  return 0;
}
