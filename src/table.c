#include "tablify.h"

// `g` contains the table dimensions.
//
// `widths` contains the maximum width of a given entry in a column,
// in utf-8 encoding.
//
// `sep` contains a char for each row, indicating if that row should
// be filled with a separator, and what separator to use.

Tuple read_table(Sv f) {
  Tuple ret = {0};
  size_t l = 0, c;

  while (f.len > 0) {
    Sv line = sv_split_escaped(&f, '\n');
    
    // a line starting with '<' means we reached the end of the table and
    // we are  in the long line references section
    if (line.data && *line.data == '<') { break; }

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

void print_table( FILE *stream) {
  for (size_t i = 0; i < g.y; i++) {
    if (sep[i]) {
      print_entry(table[i * g.x].content, width[0] == 0 ? 0 : width[0] + 1, LEFT_H,
                  stream);
      fprintf(stream, "%c", *delim);
      for (size_t j = 1; j < g.x; j++) {
        print_sep(sep[i], width[j] + 2, align[j], stream);
        fprintf(stream, "%c", *delim);
      }
    } else {
      print_entry(table[i * g.x].content, width[0] == 0 ? 0 : width[0] + 1, LEFT_H,
                  stream);
      fprintf(stream, "%c", *delim);
      for (size_t j = 1; j < g.x; j++) {
        print_entry(table[i * g.x + j].content, width[j] + 2, align[j], stream);
        fprintf(stream, "%c", *delim);
      }
    }
    fprintf(stream,"\n");
  }
}
