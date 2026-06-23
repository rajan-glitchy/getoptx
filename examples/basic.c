#include "getoptx.h"

#include <stdio.h>
#include <string.h>

typedef struct example_state {
  int verbose;
  const char *output;
} example_state;

static void
handle_option(int opt, const char *arg, void *user_data)
{
  example_state *state = (example_state *)user_data;

  switch (opt) {
  case 'v':
    state->verbose = 1;
    break;
  case 'o':
    state->output = arg;
    break;
  default:
    break;
  }
}

static void
complete_output(int opt, const char *cur, void *user_data)
{
  (void)opt;
  (void)cur;
  (void)user_data;

  printf("stdout\n");
  printf("file.txt\n");
}

static void
extra_help(void *user_data, int long_help)
{
  (void)user_data;

  printf("Example getoptx program.\n");
  if (long_help)
    printf("\nUse --completion to print a bash completion function.\n");
}

static const getoptx_option options[] = {
    {"help", 'h', GETOPTX_NO_ARG, NULL, "Show help", NULL, NULL},
    {"verbose", 'v', GETOPTX_NO_ARG, NULL, "Enable verbose output", handle_option, NULL},
    {"output", 'o', GETOPTX_REQUIRED_ARG, "PATH", "Write output to PATH", handle_option, complete_output},
    {"completion", 0, GETOPTX_NO_ARG, NULL, "Print bash completion script", NULL, NULL},
    {NULL, 0, 0, NULL, NULL, NULL, NULL},
};

int
main(int argc, char **argv)
{
  example_state state = {0, "stdout"};
  getoptx_app app = {
      "basic", NULL, "[ARGS...]", options, extra_help, &state,
  };

  if (argc >= 2 && strcmp(argv[1], "--" GETOPTX_COMPLETE_OPTION) == 0) {
    const char *prev = argc >= 3 ? argv[2] : "";
    const char *cur = argc >= 4 ? argv[3] : "";
    getoptx_complete(&app, prev, cur);
    return 0;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--completion") == 0) {
      getoptx_complete_generate(&app);
      return 0;
    }
  }

  getoptx_result result;
  if (getoptx_parse(&app, argc, argv, &result))
    return 1;

  if (result.help)
    return 0;

  printf("verbose=%d output=%s optind=%d\n", state.verbose, state.output, result.optind);
  return 0;
}
