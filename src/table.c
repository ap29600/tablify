#include "tablify.h"

// `g` contains the table dimensions.
// 
// `widths` contains the maximum width of a given entry in a column,
// in utf-8 encoding.
//
// `sep` contains a char for each row, indicating if that row should
// be filled with a separator, and what separator to use.

Geo read_table(Geo g, Sv f, Sv *table, size_t *widths, Align *align, char *sep) {
  Geo ret = {0};
  size_t l = 0, c;

  while (f.len > 0) {
    Sv line = sv_split(&f, '\n');
    ret.lines++;
    if (sep)
      sep[l] = separator_line(line);
    c = 0;
    while (line.len > 0) {
      Sv entry = sv_trim(sv_split(&line, *delim));
      if (table && widths && sep) {

        table[l * g.cols + c] = entry;

        size_t utf8len = sv_len_utf_8(entry);
        if (widths[c] < utf8len && !sep[l])
          widths[c] = utf8len;
        
        if (sep[l]) {
            Align a = get_align(entry);
            if (a) align[c] = a;
        }
      }
      line = sv_trim(line);
      c++;
    }
    if (ret.cols < c)
      ret.cols = c;
    l++;
  }
  return ret;
}

void print_table(Geo g, Sv *table, size_t *widths, Align *align, char *sep, FILE *stream) {
  for (size_t i = 0; i < g.lines; i++) {
    if (sep[i]) {
      print_entry(table[i * g.cols], widths[0] == 0 ? 0 : widths[0] + 1, stream,
                  LEFT_H);
      fprintf(stream, "%c", *delim);
      for (size_t j = 1; j < g.cols; j++) {
        print_separator(widths[j] + 2, sep[i], align[j], stream);
        printf("%c", *delim);
      }
    } else {
      print_entry(table[i * g.cols], widths[0] == 0 ? 0 : widths[0] + 1, stream,
                  LEFT_H);
      fprintf(stream, "%c", *delim);
      for (size_t j = 1; j < g.cols; j++) {
        print_entry(table[i * g.cols + j], widths[j] + 2, stream, align[j]);
        fprintf(stream, "%c", *delim);
      }
    }
    printf("\n");
  }
}

