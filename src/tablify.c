#define _XOPEN_SOURCE 700

#include <stdio.h>
#include "../lib/args.h"
#include "../lib/stringview.h"
#define TABLIFY_IMPLEMENTATION
#include "tablify.h"
#include "table.c"

int main(int argc, char **argv) {
  arg_string("--ignore", &ignore,
             "The characters to ignore when determining if a line should "
             "contain a separator.");
  arg_string("--delim", &delim, "the table delimiter");
  arg_string("--separators", &seps, "the line separator characters");
  arg_string("--input", &input, "the input file");
  arg_parse(argc, argv);

  FILE *stream = input ? fopen(input, "r") : stdin;

  Sv f = sv_slurp_stream(stream);

  // read the table once to get the dimensions
  Geo g = read_table((Geo){0}, f, 0, 0, 0, 0);

  // allocate a table for the SV's, an array for width info, and one for
  // the separator positions.

  if (!g.cols || !g.lines) {
    free(f.data);
    return 0;
  }

  Sv table[g.lines * g.cols];
  size_t width[g.cols];
  char sep[g.lines];
  Align align[g.cols];

  memset(table, 0, sizeof(table));
  memset(width, 0, sizeof(width));
  memset(sep, 0, sizeof(sep));
  memset(align, 0, sizeof(align));

  read_table(g, f, table, width, align, sep);

  print_table(g, table, width, align, sep, stdout);

  free(f.data);
}
