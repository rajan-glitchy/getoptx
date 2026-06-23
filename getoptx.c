#include "getoptx.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>

static const char *
program_name(const getoptx_app *app)
{
  return (app && app->prog_name && app->prog_name[0]) ? app->prog_name : "getoptx";
}

static int
option_count(const getoptx_option *opts)
{
  int n = 0;

  if (!opts)
    return 0;

  while (opts[n].long_name)
    n++;

  return n;
}

static int
option_value(const getoptx_option *opt, int index)
{
  return opt->short_name ? opt->short_name : 256 + index;
}

static int
validate_options(const getoptx_app *app, int count)
{
  if (count > GETOPTX_MAX_OPTIONS) {
    fprintf(stderr, "%s: too many options\n", program_name(app));
    return 1;
  }

  for (int i = 0; i < count; i++) {
    const getoptx_option *a = &app->options[i];

    if (!a->long_name[0]) {
      fprintf(stderr, "%s: invalid option name\n", program_name(app));
      return 1;
    }

    if (strcmp(a->long_name, GETOPTX_COMPLETE_OPTION) == 0) {
      fprintf(stderr, "%s: reserved option --%s\n", program_name(app), GETOPTX_COMPLETE_OPTION);
      return 1;
    }

    if (a->arg_kind != GETOPTX_NO_ARG && a->arg_kind != GETOPTX_REQUIRED_ARG) {
      fprintf(stderr, "%s: invalid arg kind for --%s\n", program_name(app), a->long_name);
      return 1;
    }

    if (a->short_name == '?' || a->short_name == ':' || a->short_name == '-') {
      fprintf(stderr, "%s: invalid short option for --%s\n", program_name(app), a->long_name);
      return 1;
    }

    for (int j = i + 1; j < count; j++) {
      const getoptx_option *b = &app->options[j];

      if (strcmp(a->long_name, b->long_name) == 0) {
        fprintf(stderr, "%s: duplicate long option --%s\n", program_name(app), a->long_name);
        return 1;
      }

      if (a->short_name && a->short_name == b->short_name) {
        fprintf(stderr, "%s: duplicate short option -%c\n", program_name(app), a->short_name);
        return 1;
      }
    }
  }

  return 0;
}

static int
find_help_value(const getoptx_option *opts)
{
  for (int i = 0; opts[i].long_name; i++) {
    if (strcmp(opts[i].long_name, "help") == 0)
      return option_value(&opts[i], i);
  }

  return -1;
}

static int
build_short_opts(const getoptx_option *opts, char *buf, size_t len)
{
  size_t pos = 0;

  if (!buf || len == 0)
    return -1;

  for (int i = 0; opts[i].long_name; i++) {
    size_t need;

    if (!opts[i].short_name)
      continue;

    need = 1 + (opts[i].arg_kind == GETOPTX_REQUIRED_ARG ? 1 : 0);
    if (pos + need + 1 > len)
      return -1;

    buf[pos++] = opts[i].short_name;

    if (opts[i].arg_kind == GETOPTX_REQUIRED_ARG)
      buf[pos++] = ':';
  }

  buf[pos] = '\0';
  return 0;
}

static const getoptx_option *
find_option_by_value(const getoptx_option *opts, int count, int value)
{
  for (int i = 0; i < count; i++) {
    if (option_value(&opts[i], i) == value)
      return &opts[i];
  }

  return NULL;
}

int
getoptx_parse(const getoptx_app *app, int argc, char **argv, getoptx_result *result)
{
  if (!app || !app->options || !app->prog_name || !argv || argc < 1) {
    fprintf(stderr, "getoptx: invalid arguments\n");
    return 1;
  }

  const int count = option_count(app->options);

  if (validate_options(app, count))
    return 1;

  if (argc >= 2 && argv[1] && strcmp(argv[1], "--" GETOPTX_COMPLETE_OPTION) == 0) {
    getoptx_complete(app, argc >= 3 && argv[2] ? argv[2] : "", argc >= 4 && argv[3] ? argv[3] : "");

    if (result) {
      result->optind = argc;
      result->help = 0;
    }

    return 0;
  }

  const int help_value = find_help_value(app->options);

  struct option long_opts[GETOPTX_MAX_OPTIONS + 1];
  for (int i = 0; i < count; i++) {
    const getoptx_option *o = &app->options[i];

    long_opts[i].name = o->long_name;
    long_opts[i].has_arg = o->arg_kind == GETOPTX_REQUIRED_ARG ? required_argument : no_argument;
    long_opts[i].flag = NULL;
    long_opts[i].val = option_value(o, i);
  }
  memset(&long_opts[count], 0, sizeof(long_opts[count]));

  char short_opts[256];
  if (build_short_opts(app->options, short_opts, sizeof(short_opts))) {
    fprintf(stderr, "%s: too many short options\n", program_name(app));
    return 1;
  }

  if (result) {
    result->optind = 1;
    result->help = 0;
  }

  optind = 1;
  opterr = 1;

  for (;;) {
    int longindex = -1;
    int c = getopt_long(argc, argv, short_opts, long_opts, &longindex);

    if (c == -1)
      break;

    if (c == '?')
      return 1;

    const getoptx_option *hit = find_option_by_value(app->options, count, c);
    if (!hit)
      continue;

    if (c == help_value) {
      getoptx_print_help(app, longindex >= 0);

      if (result) {
        result->optind = optind;
        result->help = 1;
      }

      return 0;
    }

    if (hit->handler)
      hit->handler(c, optarg, app->user_data);
  }

  if (result)
    result->optind = optind;

  return 0;
}

int
getoptx_print_help(const getoptx_app *app, int long_help)
{
  if (!app || !app->options || !app->prog_name) {
    fprintf(stderr, "getoptx: invalid arguments\n");
    return 1;
  }

  if (app->banner)
    printf("%s\n", app->banner);

  printf("Usage: %s [OPTIONS]", app->prog_name);

  if (app->usage_args && app->usage_args[0])
    printf(" %s", app->usage_args);

  printf("\n\n");

  if (app->extra_help) {
    app->extra_help(app->user_data, long_help);
    printf("\n");
  }

  printf("Options:\n");

  size_t left_width = 0;
  for (int i = 0; app->options[i].long_name; i++) {
    const getoptx_option *o = &app->options[i];
    const char *metavar = o->metavar ? o->metavar : o->long_name;
    char left[160];
    int len;

    if (o->short_name && o->arg_kind == GETOPTX_REQUIRED_ARG)
      len = snprintf(left, sizeof(left), "-%c, --%s <%s>", o->short_name, o->long_name, metavar);
    else if (o->short_name)
      len = snprintf(left, sizeof(left), "-%c, --%s", o->short_name, o->long_name);
    else if (o->arg_kind == GETOPTX_REQUIRED_ARG)
      len = snprintf(left, sizeof(left), "    --%s <%s>", o->long_name, metavar);
    else
      len = snprintf(left, sizeof(left), "    --%s", o->long_name);

    if (len > 0 && (size_t)len > left_width)
      left_width = (size_t)len;
  }

  for (int i = 0; app->options[i].long_name; i++) {
    const getoptx_option *o = &app->options[i];
    const char *metavar = o->metavar ? o->metavar : o->long_name;
    char left[160];

    if (o->short_name && o->arg_kind == GETOPTX_REQUIRED_ARG)
      snprintf(left, sizeof(left), "-%c, --%s <%s>", o->short_name, o->long_name, metavar);
    else if (o->short_name)
      snprintf(left, sizeof(left), "-%c, --%s", o->short_name, o->long_name);
    else if (o->arg_kind == GETOPTX_REQUIRED_ARG)
      snprintf(left, sizeof(left), "    --%s <%s>", o->long_name, metavar);
    else
      snprintf(left, sizeof(left), "    --%s", o->long_name);

    printf("  %-*s", (int)left_width, left);

    if (o->desc)
      printf(" %s", o->desc);

    printf("\n");
  }

  return 0;
}

static void
shell_ident(const char *s, char *buf, size_t len)
{
  size_t pos = 0;

  if (!buf || len == 0)
    return;

  buf[pos++] = '_';

  for (; s && *s && pos + 1 < len; s++) {
    char c = *s;

    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
      buf[pos++] = c;
    else
      buf[pos++] = '_';
  }

  buf[pos] = '\0';
}

void
getoptx_complete_generate(const getoptx_app *app)
{
  if (!app || !app->prog_name)
    return;

  char fn[256];
  shell_ident(app->prog_name, fn, sizeof(fn));

  printf("%s_completions() {\n", fn);
  printf("  local cur=\"${COMP_WORDS[COMP_CWORD]}\"\n");
  printf("  local prev=\"${COMP_WORDS[COMP_CWORD-1]}\"\n");
  printf("  local app=\"${COMP_WORDS[0]}\"\n");
  printf("  COMPREPLY=($(compgen -W "
         "\"$(\"$app\" --" GETOPTX_COMPLETE_OPTION " \"$prev\" \"$cur\" "
         "2>/dev/null)\" -- \"$cur\"))\n");
  printf("}\n\n");
  printf("complete -F %s_completions -- %s\n", fn, app->prog_name);
}

void
getoptx_complete(const getoptx_app *app, const char *prev, const char *cur)
{
  if (!app || !app->options)
    return;

  cur = cur ? cur : "";
  prev = prev ? prev : "";

  if (strncmp(cur, "--", 2) == 0) {
    const char *eq = strchr(cur, '=');

    if (eq && eq > cur + 2) {
      char name[128];
      size_t name_len = (size_t)(eq - (cur + 2));

      if (name_len < sizeof(name)) {
        memcpy(name, cur + 2, name_len);
        name[name_len] = '\0';

        for (int i = 0; app->options[i].long_name; i++) {
          const getoptx_option *o = &app->options[i];

          if (strcmp(o->long_name, name) == 0 && o->complete) {
            o->complete(option_value(o, i), eq + 1, app->user_data);
            return;
          }
        }
      }
    }
  }

  if (strncmp(prev, "--", 2) == 0) {
    const char *name = prev + 2;

    for (int i = 0; app->options[i].long_name; i++) {
      const getoptx_option *o = &app->options[i];

      if (strcmp(o->long_name, name) == 0 && o->complete) {
        o->complete(option_value(o, i), cur, app->user_data);
        return;
      }
    }
  }

  for (int i = 0; app->options[i].long_name; i++)
    printf("--%s\n", app->options[i].long_name);
}