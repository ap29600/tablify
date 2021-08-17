#ifndef TABLIFY_H_
#define TABLIFY_H_

#include "../lib/stringview.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// constants and parameters //////////////////////

static const char *ignore = "/:";
static const char *seps = "-=";
static const char *delim = "|";
static const char *input = NULL;
static const char *out_file = NULL;
static int compute = 0;
static size_t max_result_len = 20;

// structs and data types ////////////////////////

// defines the alignment of a cell
typedef enum {
  CENTER = 0,
  LEFT,
  LEFT_H,
  RIGHT,
  RIGHT_H,
} Align;

// holds tho values (dimensions or coordinates)
typedef struct {
  size_t x;
  size_t y;
} Tuple;

// variable size array of string views
typedef struct {
  string_view *data;
  size_t count;
  size_t cap;
} VecSV;

// evaluation state of a cell
enum eval_state { NOT_VISITED, PENDING, COMPLETE };


// a cell in the table
typedef struct {
  string_view content;
  VecSV deps;
  enum eval_state state;
} Cell;

typedef string_view Sv;

/// globals (table and friends) /////////////////////////

static Cell *table = NULL;
static size_t *width = NULL;
static Align *align = NULL;
static char *sep = NULL;
static Tuple g = {0}; // table geometry

//// function declarations /////////////

static int contains(const char *s, char c);

// index conversion
static char position_to_label_col(size_t c);
static int position_to_label_row(size_t r);
static Tuple label_to_position(string_view name);

// table processing
static char separator_line(Sv s);
static Align get_align(Sv s);
static VecSV get_deps(string_view sv);
static void dump_all_dependencies();

static Tuple read_table(Sv f);

// formatting
static int try_resolve(Tuple pos, FILE *stream);
static VecSV splice_results(string_view sv);

// function definitions ////////////////////////////////

static Align get_align(Sv s) {
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

static int contains(const char *s, char c) {
  for (size_t i = 0; s[i] != '\0'; i++)
    if (s[i] == c)
      return 1;
  return 0;
}

// return the separator character if the line is supposed to be a
// separator line, otherwise return 0.
static char separator_line(Sv s) {
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


static string_view next_ref(string_view sv) {
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
      return (string_view){.len = i, .data = sv.data};
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

static VecSV get_deps(string_view sv) {
  VecSV deps = {
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

static char position_to_label_col(size_t c) { return c - 1 + 'A'; }
static int position_to_label_row(size_t r) {
  int ret = 0;
  for (size_t i = 0; i < r; i++) {
    if (!sep[i])
      ret++;
  };
  return ret;
}

static Tuple label_to_position(string_view name) {
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

static void print_statement(char col, int row, Cell *c,
                                        FILE *stream) {
  if (!c) {
    fprintf(stderr, "Attempting to print a nonexistent cell: %c%d\n", col, row);
    return;
  }

  if (c->content.len && c->content.data[0] == '=')
    // cell has an expression as its content
    fprintf(stream, "%c%d " SV_FMT "\n", col, row, SV_ARG(c->content));
  else if (c->content.len) {
    // cell has a value as its content
    fprintf(stream, "%c%d =" SV_FMT "\n", col, row, SV_ARG(c->content));
  } else {
    // cell is empty.
    fprintf(stream, "%c%d = 0 \n", col, row);
  }

  fprintf(stream, "print( \"%c%d =\" + str(%c%d) )\n", col, row, col, row);
}

static int try_resolve(Tuple pos, FILE *stream) {
  if (pos.x >= g.x || pos.y >= g.y) {
    printf("calculating: %zu, %zu\n", pos.x, pos.y);
    printf("Cell out of bounds\n");
    return 1;
  }

  if (sep[pos.y]) {
    return 0;
  }
  Cell *c = &table[pos.y * g.x + pos.x];
  char col = position_to_label_col(pos.x);
  int row = position_to_label_row(pos.y);

  switch (c->state) {
  case NOT_VISITED:
    c->state = PENDING;

    // iterate over dependencies
    for (size_t i = 0; i < c->deps.count; i++) {
      Tuple p = label_to_position(c->deps.data[i]);
      if (try_resolve(p, stream))
        return 1;
    }

    print_statement(col, row, c, stream);

    c->state = COMPLETE;
    return 0;
  case COMPLETE:
    return 0;
  case PENDING: // dependency cycle !!
  default:
    fprintf(stderr, "Dependency cycle detected on cell %c%d\n", col, row);
    return 1;
  }
}


static VecSV splice_results(string_view sv) {
  VecSV long_svs = {0};
  while (sv.len > 0) {
    string_view line = sv_split_escaped(&sv, '\n');
    string_view cell = sv_trim(sv_split(&line, '='));
    line = sv_trim(line);

    if (line.len > max_result_len) {
      if (long_svs.count + 1 >= long_svs.cap) {
        long_svs.cap = long_svs.cap * 2 + 1;
        long_svs.data = (string_view *)realloc(long_svs.data, long_svs.cap);
      }
      long_svs.data[long_svs.count] = line;
      line.data = (char *)malloc(64);
      line.len = sprintf(line.data, "<%zu>", ++long_svs.count);
    };

    Tuple pos = label_to_position(cell);
    string_view prev_content = table[pos.y * g.x + pos.x].content;

    // only put results back in expressions that start with '='
    if (prev_content.data[0] != '=')
      continue;
    prev_content = sv_trim(sv_split_escaped(&prev_content, '#'));

    string_view spliced = {.len = prev_content.len + line.len +
                                  3, // 3 is for " # " preceding the result
                           .data = NULL};
    spliced.data =
        (char *)malloc(spliced.len + 1); // +1 for the null terminator
    if (!spliced.data) {
      fprintf(stderr, "Allocation failed\n");
      exit(1);
    }
    sprintf(spliced.data, SV_FMT " # " SV_FMT, SV_ARG(prev_content), SV_ARG(line));

    table[pos.y * g.x + pos.x].content = spliced;
    if (width[pos.x] < spliced.len)
      width[pos.x] = spliced.len;
  }
  return long_svs;
}

#endif // TABLIFY_H_
