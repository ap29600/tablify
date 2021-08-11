#include "args.h"
#include "stringview.h"
#include <stdio.h>

char *ignore = "/";
char *seps = "-=";
char *delim = "|";

typedef struct {
  size_t lines;
  size_t cols;
} geometry;

typedef enum {
  center = 0,
  left,
  left_nopad,
  right,
  right_nopad,
} align;

int good_separator(char c);
int contains(char *s, char c);
int sv_len_utf_8(string_view s);
char separator_line(string_view s);
void pad(size_t width, char fill, FILE *stream);
void print_entry(string_view s, size_t width, FILE *stream, align align);
string_view slurp_stream(FILE *stream);
geometry get_table_geometry(string_view f);
void read_table(geometry g, string_view f, string_view *table, size_t *width,
                char *separators);
void print_table(geometry g, string_view *table, size_t *width,
                 char *separators, FILE *stream);

int main(int argc, char **argv) {

  arg_string("-ignore", &ignore,
             "The characters to ignore when determining if a line should "
             "contain a separator.");
  arg_string("-delim", &delim, "the table delimiter");
  arg_string("-separators", &seps, "the line separator characters");

  arg_parse(argc, argv);

  string_view f = slurp_stream(stdin);

  // read the table once to get the dimensions
  geometry g = get_table_geometry(f);

  // allocate a table for the SV's, an array for width info, and one for
  // the separator positions.
  string_view *table = calloc(g.lines * g.cols * sizeof(string_view) + // table
                                  g.cols * sizeof(size_t) +            // widths
                                  g.lines * sizeof(char), // separators
                              1);
  size_t *width = (size_t *)(table + g.lines * g.cols);
  char *separators = (char *)(width + g.cols);

  read_table(g, f, table, width, separators);

  print_table(g, table, width, separators, stdout);

  free(f.data);
  free(table); // this will also free the other arrays
}

int good_separator(char c) { return c == '-' || c == '='; }

int contains(char *s, char c) {
  for (size_t i = 0; s[i] != '\0'; i++)
    if (s[i] == c)
      return 1;
  return 0;
}

int sv_len_utf_8(string_view s) {
  int ret = 0;
  for (; s.len > 0; s.len--)
    if (((*s.data++) & 0xc0) != 0x80)
      ret++;
  return ret;
}

// return the separator character if the line is supposed to be a
// separator line, otherwise return 0.
char separator_line(string_view s) {
  char sep = '\0';
  for (size_t i = 0; i < s.len; i++)
    if (!sep && contains(seps, s.data[i]))
      sep = s.data[i];
    else if (isspace(s.data[i]) || s.data[i] == sep ||
             s.data[i] == *delim || contains(ignore, s.data[i]))
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

void print_entry(string_view s, size_t width, FILE *stream, align align) {
  size_t pad_left = 0, pad_right = 0, utf8len = sv_len_utf_8(s);

  switch (align) {
  case left:
    pad_left = 1;
    pad_right = width - utf8len - pad_left;
    break;
  case left_nopad:
    pad_left = 0;
    pad_right = width - utf8len - pad_left;
    break;
  case center:
    pad_left = (width - utf8len) / 2;
    pad_right = width - utf8len - pad_left;
    break;
  case right:
    pad_right = 1;
    pad_left = width - utf8len - pad_right;
    break;
  case right_nopad:
    pad_right = 0;
    pad_left = width - utf8len - pad_right;
    break;
  }

  pad(pad_left, ' ', stream);
  fprintf(stream, "" SV_FMT "", SV_ARG(s));
  pad(pad_right, ' ', stream);
}

string_view slurp_stream(FILE *stream) {
  string_view ret = {0};
  size_t capacity = 0;
  char c;

  while (EOF != (c = getc(stream))) {
    if (capacity < ret.len + 1) {
      capacity = (capacity == 0) ? 1024 : capacity * 2;
      ret.data = realloc(ret.data, capacity);
      if (!ret.data)
        return (string_view){0};
    }

    ret.data[ret.len++] = c;
  }
  return ret;
}

geometry get_table_geometry(string_view f) {
  geometry ret = {0};

  while (f.len > 0) {
    string_view line = sv_split(&f, '\n');
    ret.lines++;
    size_t c = 0;

    while (line.len > 0) {
      sv_split(&line, *delim);
      c++; // ignore the part following the last delimiter
    }
    if (ret.cols < c)
      ret.cols = c;
  }

  return ret;
}

void read_table(geometry g, string_view f, string_view *table, size_t *width,
                char *separators) {
  size_t l = 0, c;
  while (f.len > 0) {
    string_view line = sv_split(&f, '\n');
    separators[l] = separator_line(line);
    c = 0;
    while (line.len > 0) {
      table[l * g.cols + c] = sv_trim(sv_split(&line, *delim));
      size_t utf8len = sv_len_utf_8(table[l * g.cols + c]);
      if (width[c] < utf8len && !separators[l])
        width[c] = utf8len;
      c++;
    }
    l++;
  }
}

void print_table(geometry g, string_view *table, size_t *width,
                 char *separators, FILE *stream) {
  for (size_t i = 0; i < g.lines; i++) {
    if (separators[i]) {
      print_entry(table[i * g.cols], width[0] == 0 ? 0 : width[0] + 1, stream,
                  left_nopad);
      fprintf(stream, "%c", *delim);
      for (size_t j = 1; j < g.cols; j++) {
        pad(width[j] + 2, separators[i], stream);
        printf("%c", *delim);
      }
    } else {
      print_entry(table[i * g.cols], width[0] == 0 ? 0 : width[0] + 1, stream,
                  left_nopad);
      fprintf(stream, "%c", *delim);
      for (size_t j = 1; j < g.cols; j++) {
        print_entry(table[i * g.cols + j], width[j] + 2, stream, center);
        fprintf(stream, "%c", *delim);
      }
    }
    printf("\n");
  }
}
