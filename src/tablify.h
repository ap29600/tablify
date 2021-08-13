#ifndef TABLIFY_H_
#define TABLIFY_H_

#include "../lib/stringview.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const char *ignore = "/:";
static const char *seps = "-=";
static const char *delim = "|";
static const char *input = NULL;
static int compute = 0;

typedef enum {
  CENTER = 0,
  LEFT,
  LEFT_H,
  RIGHT,
  RIGHT_H,
} Align;

typedef struct {
  size_t x;
  size_t y;
} Tuple;

typedef struct {
  string_view *data;
  size_t count;
  size_t cap;
} Deps;

typedef struct {
  string_view sv;
  Deps deps;
  enum { NOT_VISITED, PENDING, COMPLETE } state;
} Cell;

typedef string_view Sv;

static Cell   *table = NULL;
static size_t *width = NULL;
static Align  *align = NULL;
static char   *sep   = NULL;
static Tuple g = {0}; // table geometry

int contains(const char *s, char c);

// index conversion 
char col_of(size_t c);
int row_of(size_t r);
Tuple pos_of(string_view name);

// table processing
char separator_line(Sv s);
Align get_align(Sv s);
Deps get_deps(string_view sv);
void dump_all_dependencies();

Tuple read_table(Sv f);

// formatting
void pad(size_t width, char fill, FILE *stream);
void print_sep(char fill, size_t width, Align align, FILE *stream);
void print_entry(Sv s, size_t width, Align align, FILE *stream);
void print_table(FILE *stream);
void print_computation_statement(char col, int row, Cell *c, FILE *stream);
void print_computation_steps(FILE *stream);
int try_resolve(Tuple pos, FILE *stream);

#endif // TABLIFY_H_

#ifdef TABLIFY_IMPLEMENTATION
#undef TABLIFY_IMPLEMENTATION

void print_sep(char fill, size_t width, Align align, FILE *stream) {
  size_t sep_width = width;
  switch (align) {
  case LEFT:
  case LEFT_H:
    sep_width -= 1;
    fputc(':', stream);
    break;
  case RIGHT:
  case RIGHT_H:
    sep_width -= 1;
    break;
  case CENTER:
    break;
  }
  pad(sep_width, fill, stream);

  if (align == RIGHT || align == RIGHT_H)
    fputc(':', stream);
}

Align get_align(Sv s) {
  if (s.data[0] == ':') {
    if (s.data[s.len - 1] == ':')
      return CENTER;
    else
      return LEFT;
  } else if (s.data[s.len - 1] == ':')
    return RIGHT;
  else
    return CENTER;
}

int contains(const char *s, char c) {
  for (size_t i = 0; s[i] != '\0'; i++)
    if (s[i] == c)
      return 1;
  return 0;
}

// return the separator character if the line is supposed to be a
// separator line, otherwise return 0.
char separator_line(Sv s) {
  char sep = '\0';
  for (size_t i = 0; i < s.len; i++)
    if (!sep && contains(seps, s.data[i]))
      sep = s.data[i];
    else if (isspace(s.data[i]) || s.data[i] == sep || s.data[i] == *delim ||
             contains(ignore, s.data[i]))
      continue;
    else
      return 0;
  return sep;
}

void pad(size_t width, char fill, FILE *stream) {
  for (size_t i = 0; i < width; i++) {
    fprintf(stream, "%c", fill);
  }
}

void print_entry(Sv s, size_t width, Align align, FILE *stream) {
  size_t pad_left = 0, pad_right = 0, utf8len = sv_len_utf_8(s);

  switch (align) {
  case LEFT:
    pad_left = 1;
    pad_right = width - utf8len - pad_left;
    break;
  case LEFT_H:
    pad_left = 0;
    pad_right = width - utf8len - pad_left;
    break;
  case CENTER:
    pad_left = (width - utf8len) / 2;
    pad_right = width - utf8len - pad_left;
    break;
  case RIGHT:
    pad_right = 1;
    pad_left = width - utf8len - pad_right;
    break;
  case RIGHT_H:
    pad_right = 0;
    pad_left = width - utf8len - pad_right;
    break;
  }

  pad(pad_left, ' ', stream);
  fprintf(stream, "" SV_FMT "", SV_ARG(s));
  pad(pad_right, ' ', stream);
}

string_view next_ref(string_view sv) {
  while (sv.len > 0) {

    int letter = 0;
    int number = 0;

    // only support up to 26 columns for now :,(
    if (isupper(*sv.data) && sv.len > 0) {
      letter = 1;
    }

    size_t i;
    for (i = 1; i < sv.len && letter && isdigit(sv.data[i]); i++)
      number = 1;

    // if we parsed a number and the next char does not
    // invalidate it:
    if (number && !isalnum(sv.data[i])) {
      // we're done
      return (string_view){.data = sv.data, .len = i};
    }

    sv.data += i;
    sv.len -= i;

    // otherwise find the next candidate
    while (!isupper(*sv.data) && sv.len > 0) {
      sv.data++;
      sv.len--;
    }
  }
  return (string_view){0};
}

Deps get_deps(string_view sv) {
  Deps deps = {
      .data = (string_view *)malloc(8 * sizeof(string_view)),
      .count = 0,
  };
  size_t cap = 8;
  sv.data++; // get rid of the '=' char
  sv.len--;

  while (sv.len > 0) {

    string_view dep = next_ref(sv);

    if (dep.len == 0) {
      // no more dependencies
      return deps;
    }

    // grow the vector, we need to add a dependency
    if (cap <= deps.count) {
      cap *= 2;
      deps.data = (string_view *)realloc(deps.data, cap * sizeof(string_view));
    }

    deps.data[deps.count++] = dep;

    // we took a chunk out, fix sv.
    //    (last of sv + 1)  - (last of dep + 1)
    sv.len = (sv.data + sv.len) - (dep.data + dep.len);
    sv.data = dep.data + dep.len;
  }

  return deps;
}

char col_of(size_t c) { return c - 1 + 'A'; }
int row_of(size_t r) {
  int ret = 0;
  for (size_t i = 0; i < r; i++) {
    if (!sep[i])
      ret++;
  };
  return ret;
}

Tuple pos_of(string_view name) {
  Tuple ret = {0};
  ret.x = name.data[0] - 'A' + 1;
  size_t y = 0;
  for (size_t i = 1; i < name.len; i++)
    y = y * 10 + name.data[i] - '0';

  ret.y = 0;
  for (size_t i = 0; i <= y; ret.y++) {
    if (!sep[ret.y])
      i++;
  }
  ret.y--;

  return ret;
}

void print_computation_statement(char col, int row, Cell *c, FILE *stream) {
  if (!c) {
    fprintf(stream, "%c%d = 0 \n", col, row);
  }
  // print to file
  if (c->sv.len && c->sv.data[0] == '=') // print expression
    fprintf(stream, "%c%d " SV_FMT "\n", col, row, SV_ARG(c->sv));
  else if (c->sv.len) {
    fprintf(stream, "%c%d =" SV_FMT "\n", col, row, SV_ARG(c->sv));
  } else {
    fprintf(stream, "%c%d = 0 \n", col, row);
  }

  fprintf(stream, "print( \"%c%d =\" + str(%c%d) )\n", col, row, col, row);
}

void print_deps(Deps v) {
  for (size_t i = 0; i < v.count; i++)
    printf("  ref{ " SV_FMT " }\n", SV_ARG(v.data[i]));
}

void dump_all_dependencies() {
  for (size_t i = 0; i < g.y; i++)
    for (size_t j = 0; j < g.x; j++) {
      printf("Cell %zu, %zu: \n", j, i);
      print_deps(table[i * g.x + j].deps);
    }
}

int try_resolve(Tuple pos, FILE *stream) {
  if (pos.x >= g.x || pos.y >= g.y) {
    printf("calculating: %zu, %zu\n", pos.x, pos.y);
    printf("Cell out of bounds\n");
    return 1;
  }

  if (sep[pos.y]) {
    return 0;
  }
  Cell *c = &table[pos.y * g.x + pos.x];
  char col = col_of(pos.x);
  int row = row_of(pos.y);

  switch (c->state) {
  case NOT_VISITED:
    c->state = PENDING;

    // iterate over dependencies
    for (size_t i = 0; i < c->deps.count; i++) {
      Tuple p = pos_of(c->deps.data[i]);
      // printf("as dep: %c%d (%zu, %zu)\n", col_of(p.x), row_of(sep, p.y), p.x, p.y);
      if (try_resolve(p, stream))
        return 1;
    }

    print_computation_statement(col, row, c, stream);

    c->state = COMPLETE;
    return 0;
  case COMPLETE:
    return 0;
  case PENDING: // dependency cycle !!
  default:
    return 1;
  }
}

void print_computation_steps(FILE *stream) {
  fprintf (stream, "import math as m\n");
  for (size_t x = 1; x < g.x; x++)
    for (size_t y = 0; y < g.y; y++) {
      if (!table[y * g.x + x].sv.data || table[y * g.x + x].sv.data[0] != '=')
        continue;
      if (try_resolve((Tuple){.x = x, .y = y}, stream)) {
        printf("dependency cycle detected\n");
        return;
      }
    }
}


#endif // TABLIFY_IMPLEMENTATION
