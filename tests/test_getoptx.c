#include "getoptx.h"

#include <stdio.h>
#include <string.h>

typedef struct test_state {
  int verbose;
  int mode_seen;
  int mode_opt;
  const char *output;
} test_state;

static void
handle_option(int opt, const char *arg, void *user_data)
{
  test_state *state = (test_state *)user_data;

  switch (opt) {
  case 'v':
    state->verbose = 1;
    break;
  case 'o':
    state->output = arg;
    break;
  default:
    state->mode_seen = 1;
    state->mode_opt = opt;
    state->output = arg;
    break;
  }
}

static const getoptx_option options[] = {
    {"help", 'h', GETOPTX_NO_ARG, NULL, "Show help", NULL, NULL},
    {"verbose", 'v', GETOPTX_NO_ARG, NULL, "Enable verbose output", handle_option, NULL},
    {"output", 'o', GETOPTX_REQUIRED_ARG, "PATH", "Write output to PATH", handle_option, NULL},
    {"mode", 0, GETOPTX_REQUIRED_ARG, "NAME", "Set mode", handle_option, NULL},
    {NULL, 0, 0, NULL, NULL, NULL, NULL},
};

static int
expect_int(const char *name, int got, int want)
{
  if (got == want)
    return 0;

  fprintf(stderr, "%s: got %d, want %d\n", name, got, want);
  return 1;
}

static int
expect_string(const char *name, const char *got, const char *want)
{
  if (got && strcmp(got, want) == 0)
    return 0;

  fprintf(stderr, "%s: got %s, want %s\n", name, got ? got : "(null)", want);
  return 1;
}

static int
test_parse_short_and_long(void)
{
  char *argv[] = {"prog", "-v", "--output", "out.txt", "positional", NULL};
  test_state state = {0, 0, 0, NULL};
  getoptx_app app = {"prog", NULL, "[ARGS...]", options, NULL, &state};
  getoptx_result result;

  if (getoptx_parse(&app, 5, argv, &result))
    return 1;

  return expect_int("verbose", state.verbose, 1) || expect_string("output", state.output, "out.txt") ||
         expect_int("optind", result.optind, 4) || expect_int("help", result.help, 0);
}

static int
test_parse_long_only_value(void)
{
  char *argv[] = {"prog", "--mode", "fast", NULL};
  test_state state = {0, 0, 0, NULL};
  getoptx_app app = {"prog", NULL, NULL, options, NULL, &state};
  getoptx_result result;

  if (getoptx_parse(&app, 3, argv, &result))
    return 1;

  return expect_int("mode_seen", state.mode_seen, 1) || expect_int("mode_opt", state.mode_opt, 256 + 3) ||
         expect_string("mode", state.output, "fast") || expect_int("optind", result.optind, 3);
}

int
main(void)
{
  if (test_parse_short_and_long())
    return 1;

  if (test_parse_long_only_value())
    return 1;

  return 0;
}
