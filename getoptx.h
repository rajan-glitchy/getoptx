#ifndef GETOPTX_H
#define GETOPTX_H

/*

* getoptx is a small table-driven wrapper around getopt_long().
*
* Callers provide a null-terminated getoptx_option array, optional callbacks
* for option-specific parsing and completion, and an opaque user_data pointer.
* getoptx can parse argc/argv, print a basic help screen, and list completion
* candidates for shell completion.
*
* getoptx_complete_generate() emits a Bash completion function that invokes
* the program with:
*
* ```
  --__getoptx_complete PREV CUR
  ```
*
* The completion option name is exposed as GETOPTX_COMPLETE_OPTION.
* getoptx_parse() handles this internal option automatically before normal
* option parsing and dispatches completion requests to getoptx_complete().
*
* This library requires getopt_long(), which is available on GNU/POSIX-like
* systems and many libc implementations, but is not part of ISO C.
  */

#include <stddef.h>

#define GETOPTX_MAX_OPTIONS 64
#define GETOPTX_COMPLETE_OPTION "__getoptx_complete"

typedef enum {
  GETOPTX_NO_ARG = 0,
  GETOPTX_REQUIRED_ARG = 1,
} getoptx_arg_kind;

typedef struct getoptx_option {
  const char *long_name;
  char short_name;
  getoptx_arg_kind arg_kind;
  const char *metavar;
  const char *desc;
  void (*handler)(int opt, const char *arg, void *user_data);
  void (*complete)(int opt, const char *cur, void *user_data);
} getoptx_option;

typedef struct getoptx_app {
  const char *prog_name;
  const char *banner;
  const char *usage_args;
  const getoptx_option *options;
  void (*extra_help)(void *user_data, int long_help);
  void *user_data;
} getoptx_app;

typedef struct getoptx_result {
  int optind;
  int help;
} getoptx_result;

int getoptx_parse(const getoptx_app *app, int argc, char **argv, getoptx_result *result);

int getoptx_print_help(const getoptx_app *app, int long_help);

void getoptx_complete(const getoptx_app *app, const char *prev, const char *cur);

void getoptx_complete_generate(const getoptx_app *app);

#endif /* GETOPTX_H */
