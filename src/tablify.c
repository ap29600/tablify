#define _XOPEN_SOURCE 700
#include "../lib/args.h"
#include "../lib/stringview.h"
#include <stdio.h>
#define TABLIFY_IMPLEMENTATION
#include "io.h"
#include "tablify.h"

int main(int argc, char **argv) {
  arg_string("--ignore", &ignore,
             "The characters to ignore when determining if a line should "
             "contain a separator.");
  arg_string("--delim", &delim, "the table delimiter");
  arg_string("--separators", &seps, "the line separator characters");
  arg_string("--input", &input, "the input file");
  arg_string("--output", &out_file, "the output file");
  int help = 0;
  arg_int("--help", &help, "print this help message");
  arg_int("--compute", &compute,
          "perform computations in cells that start with '='");
  if (arg_parse(argc, argv))
    return 1;

  if (help) {
    arg_print_usage(stdout);
    return 0;
  }

  Sv f;
  if (input) {
    FILE *stream = fopen(input, "r");
    f = sv_slurp_file(stream);
    fclose(stream);
  } else {
    f = sv_slurp_stream(stdin);
  }

  // read the table once to get the dimensions.
  // since all the global pointers are NULL
  // nothing is written.
  g = read_table(f);

  if (!g.x || !g.y) {
    free(f.data);
    return 0;
  }

  // allocate a table for the SV's, an array for width info, and one for
  // the separator positions.
  table = calloc(g.x * g.y, sizeof(Cell));
  width = calloc(g.x, sizeof(size_t));
  align = calloc(g.x, sizeof(Align));
  sep = calloc(g.y, 1);

  // read the table again, this time saving the cells.
  read_table(f);

  // dump_deps(g, table);

  VecSV long_entries = {0};
  if (compute) {
    FILE *py = fopen("py.txt", "w");
    print_computation_steps(py);
    fclose(py);

    FILE *p_out = popen("python py.txt", "r");

    if (!p_out) {
      printf("failure to get pipe\n");
    }

    string_view out = sv_slurp_stream(p_out);
    pclose(p_out);

    long_entries = splice_results(out);

    // free(out.data);
  }

  FILE *output;
  if (out_file)
    output = fopen(out_file, "w");
  else
    output = stdout;
  print_table(output);
  print_long_entries(long_entries, output);

  // cleanup
  free(table);
  free(width);
  free(align);
  free(sep);
  free(f.data);
}
