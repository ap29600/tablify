#ifndef ARGS_H_
#define ARGS_H_

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void arg_int(const char *name, int *value, const char *def);
void arg_float(const char *name, float *value, const char *def);
void arg_string(const char *name, const char ** value, const char *def);

int arg_parse(int argc, char **argv);
void arg_print_usage(FILE *stream);

#endif // ARGS_H_

///////////////////////////

#ifndef ARGS_IMPLEMENTATION
#define ARGS_IMPLEMENTATION

#ifndef ARGS_COUNT
#define ARGS_COUNT 256
#endif

enum arg_h_type {
  ARG_TYPE_UNINIT = 0,
  ARG_TYPE_INT,
  ARG_TYPE_FLOAT,
  ARG_TYPE_STRING
};

union arg_h_union {
  int *Int;
  const char **String;
  float *Float;
};

typedef struct {
  enum arg_h_type type;
  union arg_h_union as;
  const char *name;
  const char *def;
} Arg;

static struct {
  Arg a[ARGS_COUNT];
  size_t num;
} args;

inline void arg_int(const char *name, int *value, const char *def) {
  if (args.num + 1 >= ARGS_COUNT) {
    fprintf(stderr, "Too many arguments were declared\n");
    exit(0);
  }
  args.a[args.num] = (Arg){
      .type = ARG_TYPE_INT,
      .as.Int = value,
      .name = name,
      .def = def,
  };
  args.num++;
}

inline void arg_float(const char *name, float *value, const char *def) {
  if (args.num + 1 >= ARGS_COUNT) {
    fprintf(stderr, "Too many arguments were declared\n");
    exit(0);
  }
  args.a[args.num] = (Arg){
      .type = ARG_TYPE_FLOAT,
      .as.Float = value,
      .name = name,
      .def = def,
  };
  args.num++;
}

inline void arg_string(const char *name, const char **value, const char *def) {
  if (args.num + 1 >= ARGS_COUNT) {
    fprintf(stderr, "Too many arguments were declared\n");
    exit(0);
  }
  args.a[args.num] = (Arg){
      .type = ARG_TYPE_STRING,
      .as.String = value,
      .name = name,
      .def = def,
  };
  args.num++;
}

static inline int arg_parse_at(char *source, Arg a) {
  int tmp_int;
  float tmp_float;
  char *endptr = NULL;
  switch (a.type) {
  case ARG_TYPE_INT:
    tmp_int = strtol(source, &endptr, 0);
    if (*endptr) {
      fprintf(stderr,
              "Argument after \"%s\" must be an integer, found "
              "\"%s\" instead\n",
              a.name, source);
      return 1;
    }
    *a.as.Int = tmp_int;
    break;

  case ARG_TYPE_FLOAT:
    tmp_float = strtof(source, &endptr);
    if (*endptr) {
      fprintf(stderr,
              "Argument after \"%s\" must be a floating point number, found "
              "\"%s\" instead\n",
              a.name, source);
      return 1;
    }
    *a.as.Float = tmp_float;
    break;

  case ARG_TYPE_STRING:
    *a.as.String = source;
    break;

  case ARG_TYPE_UNINIT:
    exit(0);
  }

  return 0;
}

static inline int is_eq_arg(const char *name, const char *buf) {
  int i;
  for (i = 0; name[i] != '\0'; i++)
    if (name[i] != buf[i])
      return 0;
  return buf[i] == '=' ? 1 : 0;
}

inline int arg_parse(int argc, char **argv) {

  enum { parsed, available } state[argc];
  for (int i = 1; i < argc; i++)
    state[i] = available;
  state[0] = parsed;

  for (size_t i = 0; i < args.num; i++) {
    for (int j = 0; j < argc; j++) {
      if (state[j] == available && strcmp(args.a[i].name, argv[j]) == 0) {
        state[j] = parsed;
        if (args.a[i].type == ARG_TYPE_UNINIT) {
          fprintf(stderr, "Tried to parse uninitialized argument");
          return 1;
        }

        if (j == argc - 1) {
          fprintf(stderr, "value expected after \"%s\"\n", args.a[i].name);
          return 1;
        }

        if (state[j + 1] == parsed) {
          fprintf(stderr, "Tried to parse the same argument twice (%s)\n",
                  argv[j + 1]);
          return 1;
        }
        state[j + 1] = parsed;
        if (arg_parse_at(argv[j + 1], args.a[i]))
          return 1;
      } else if (state[j] == available && is_eq_arg(args.a[i].name, argv[j])) {
        state[j] = parsed;
        if (args.a[i].type == ARG_TYPE_UNINIT) {
          fprintf(stderr, "Tried to parse uninitialized argument");
          return 1;
        }

        if (arg_parse_at(argv[j] + strlen(args.a[i].name) + 1, args.a[i]))
          return 1;
      }
    }
  }

  for (int i = 1; i < argc; i++) {
    if (state[i] == available) {
      fprintf(stderr, "WARNING: unrecognized argument %s\n", argv[i]);
    }
  }

  return 0;
}

inline void arg_print_usage(FILE *stream) {
  fprintf(stream, "Usage:\n");
  for (size_t i = 0; i < args.num; i++) {
    Arg a = args.a[i];
    switch (a.type) {
    case ARG_TYPE_INT:
      fprintf(stream,
              "\t%s <int>\n"
              "\t\t%s\n",
              a.name, a.def);
      break;
    case ARG_TYPE_FLOAT:
      fprintf(stream,
              "\t%s <float>\n"
              "\t\t%s\n",
              a.name, a.def);
      break;
    case ARG_TYPE_STRING:
      fprintf(stream,
              "\t%s <string>\n"
              "\t\t%s\n",
              a.name, a.def);
      break;

    case ARG_TYPE_UNINIT:
      fprintf(stderr, "ERROR: Tried to print usage of uninitialized arg\n");
      exit(1);
      break;
    }
  }
}

#endif // ARGS_IMPLEMENTATION
