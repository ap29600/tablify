#ifndef IO_H_
#define IO_H_

#include "tablify.h"

////////////////////////////////// DECLARATIONS ///////////////////////////////

// subroutines
static void print_long_entries(VecSV vec, FILE *stream);
static void print_sep(char fill, size_t width, Align align, FILE *stream);
static void print_entry(Sv s, size_t width, Align align, FILE *stream);
static void pad(size_t width, char fill, FILE *stream);

// print to intermediate python file
static void print_statement(char col, int row, Cell *c, FILE *stream);
static void print_computation_steps(FILE *stream);

// read-write whole table
static Tuple read_table(Sv f);
static void print_table(FILE *stream);

////////////////////////////////// DEFINITIONS ////////////////////////////////

// prints a separator entry with correct width and alignment markers
static void print_sep(char fill, size_t width, Align align, FILE *stream) {
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

// print 'width' copies of 'fill' to 'stream'
static void pad(size_t width, char fill, FILE *stream) {
  for (size_t i = 0; i < width; i++) {
    fprintf(stream, "%c", fill);
  }
}

// for each of the entries in vec, print a '<#>' marker and the value.
static void print_long_entries(VecSV vec, FILE *stream) {
  for (size_t i = 0; i < vec.count; i++) {
    fprintf(stream, "<%zu>: " SV_FMT "\n", i + 1, SV_ARG(vec.data[i]));
  }
}

// print an entry to 'stream' with padding.
static void print_entry(Sv s, size_t width, Align align, FILE *stream) {
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

// reads the whole table from a string_view
static Tuple read_table(Sv f) {
  Tuple ret = {0};
  size_t l = 0, c;

  while (f.len > 0) {
    Sv line = sv_split_escaped(&f, '\n');

    // a line starting with '<' means we reached the end of the table and
    // we are  in the long line references section
    if (line.data && *line.data == '<') {
      break;
    }

    ret.y++;
    if (sep)
      sep[l] = separator_line(line);
    c = 0;
    while (line.len > 0) {
      Sv entry = sv_trim(sv_split_escaped(&line, *delim));
      if (table && width && sep) {

        table[l * g.x + c].content = entry;
        if (*entry.data == '=' && !sep[l])
          table[l * g.x + c].deps = get_deps(entry);

        size_t utf8len = sv_len_utf_8(entry);
        if (width[c] < utf8len && !sep[l])
          width[c] = utf8len;

        if (sep[l]) {
          Align a = get_align(entry);
          if (a)
            align[c] = a;
        }
      }
      line = sv_trim(line);
      c++;
    }
    if (ret.x < c)
      ret.x = c;
    l++;
  }
  return ret;
}

// prints the whole table with correct formatting
static void print_table(FILE *stream) {
  for (size_t i = 0; i < g.y; i++) {
    if (sep[i]) {
      print_entry(table[i * g.x].content, width[0] == 0 ? 0 : width[0] + 1,
                  LEFT_H, stream);
      fprintf(stream, "%c", *delim);
      for (size_t j = 1; j < g.x; j++) {
        print_sep(sep[i], width[j] + 2, align[j], stream);
        fprintf(stream, "%c", *delim);
      }
    } else {
      print_entry(table[i * g.x].content, width[0] == 0 ? 0 : width[0] + 1,
                  LEFT_H, stream);
      fprintf(stream, "%c", *delim);
      for (size_t j = 1; j < g.x; j++) {
        print_entry(table[i * g.x + j].content, width[j] + 2, align[j], stream);
        fprintf(stream, "%c", *delim);
      }
    }
    fprintf(stream, "\n");
  }
}

// generate the python file for computation
static void print_computation_steps(FILE *stream) {
  fprintf(stream, "import math as m\n");
  for (size_t x = 1; x < g.x; x++)
    for (size_t y = 0; y < g.y; y++) {
      if (!table[y * g.x + x].content.data ||
          table[y * g.x + x].content.data[0] != '=')
        continue;
      if (try_resolve((Tuple){.x = x, .y = y}, stream)) {
        printf("dependency cycle detected\n");
        return;
      }
    }
}

/////////////////// DEBUG /////////////////////////////////

// inspect the dependencies in the vecotor
static void print_deps(VecSV v) {
  for (size_t i = 0; i < v.count; i++)
    printf("  ref{ " SV_FMT " }\n", SV_ARG(v.data[i]));
}

// print the dependencies of every cell
static void dump_all_dependencies() {
  for (size_t i = 0; i < g.y; i++)
    for (size_t j = 0; j < g.x; j++) {
      printf("Cell %zu, %zu: \n", j, i);
      print_deps(table[i * g.x + j].deps);
    }
}

#endif // IO_H_
