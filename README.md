# getoptx

`getoptx` is a small table-driven C wrapper around `getopt_long()`.

It is meant for compact command-line tools that want:

- long and short option parsing from one option table
- per-option handler callbacks
- generated help text
- optional bash completion support
- no heap allocation

`getoptx` requires `getopt_long()`. It targets GNU/POSIX-like C environments and
is not ISO C-only.

## Files

Copy these into your project:

```text
getoptx.c
getoptx.h
```

Then compile `getoptx.c` with your program.

## Build

```sh
make
make check
```

The example program is built at:

```sh
examples/basic
```

## Basic Usage

Define a null-terminated option table:

```c
#include "getoptx.h"

static void
handle_option(int opt, const char *arg, void *user_data)
{
  switch (opt) {
  case 'v':
    /* --verbose / -v */
    break;
  case 'o':
    /* --output / -o, value is in arg */
    break;
  default:
    break;
  }
}

static const getoptx_option options[] = {
  {"help", 'h', GETOPTX_NO_ARG, NULL, "Show help", NULL, NULL},
  {"verbose", 'v', GETOPTX_NO_ARG, NULL, "Enable verbose output", handle_option, NULL},
  {"output", 'o', GETOPTX_REQUIRED_ARG, "PATH", "Write output to PATH", handle_option, NULL},
  {NULL, 0, 0, NULL, NULL, NULL, NULL},
};
```

Parse `argc` and `argv`:

```c
getoptx_app app = {
  "mytool",
  NULL,
  "[ARGS...]",
  options,
  NULL,
  NULL,
};
getoptx_result result;

if (getoptx_parse(&app, argc, argv, &result))
  return 1;

if (result.help)
  return 0;
```

If an option has no short name, `getoptx` passes a generated option value to its
handler. The generated value is `256 + option_index`.

## Help

An option named `help` is treated as the built-in help option. `-h` and `--help`
both print help and set `result.help`.

The optional `extra_help` callback is printed between the usage line and the
option list. Its `long_help` argument is `1` for `--help` and `0` for `-h`.

## Completion

`getoptx` includes two optional helpers:

```c
void getoptx_complete_generate(const getoptx_app *app);
void getoptx_complete(const getoptx_app *app, const char *prev, const char *cur);
```

`getoptx_complete_generate()` prints a bash completion function. That function
calls your program with:

```text
--__getoptx_complete PREV CUR
```

Your program must route that hidden option to `getoptx_complete()` before normal
option parsing:

```c
if (argc >= 2 && strcmp(argv[1], "--" GETOPTX_COMPLETE_OPTION) == 0) {
  const char *prev = argc >= 3 ? argv[2] : "";
  const char *cur = argc >= 4 ? argv[3] : "";
  getoptx_complete(&app, prev, cur);
  return 0;
}
```

Option-specific completion callbacks receive the same `opt` value that handlers
receive:

```c
void (*complete)(int opt, const char *cur, void *user_data);
```

## Limits

- Maximum options: `GETOPTX_MAX_OPTIONS`, currently `64`.
- Option tables must be terminated by an entry whose `long_name` is `NULL`.
- Duplicate short names and duplicate long names are rejected.
- `getoptx_parse()` resets `optind` to `1` before parsing. This matches common
  GNU/POSIX usage; unusual libc implementations may differ.

## License

MIT.
