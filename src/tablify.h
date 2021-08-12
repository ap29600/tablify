#ifndef TABLIFY_H_
#define TABLIFY_H_

#include "../lib/stringview.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const char *ignore = "/:";
static const char *seps = "-=";
static const char *delim = "|";
static const char *input = NULL;

typedef struct {
  size_t lines;
  size_t cols;
} Geo;

typedef enum {
  CENTER = 0,
  LEFT,
  LEFT_H,
  RIGHT,
  RIGHT_H,
} Align;

typedef string_view Sv;

Align get_align(Sv s);
int contains(const char *s, char c);
char separator_line(Sv s);
void pad(size_t width, char fill, FILE *stream);
void print_separator(size_t width, char fill, Align align, FILE *stream);
void print_entry(Sv s, size_t width, FILE *stream, Align align);
Geo read_table(Geo g, Sv f, Sv *table, size_t *width, Align *align, char *sep);
void print_table(Geo g, Sv *table, size_t *width, Align *align,
                 char *separators, FILE *stream);

int sv_len_utf_8(Sv s);
Sv sv_slurp_stream(FILE *stream);

#endif // TABLIFY_H_

#ifdef TABLIFY_IMPLEMENTATION
#undef TABLIFY_IMPLEMENTATION

void print_separator(size_t width, char fill, Align align,
                            FILE *stream) {
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

int sv_len_utf_8(Sv s) {
  int ret = 0;
  for (; s.len > 0; s.len--)
    if (((*s.data++) & 0xc0) != 0x80)
      ret++;
  return ret;
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

void print_entry(Sv s, size_t width, FILE *stream, Align align) {
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

Sv sv_slurp_stream(FILE *stream) {
  Sv ret = {0};
  size_t capacity = 0;
  if (isatty(fileno(stream))) {
    char c;
    while (EOF != (c = getc(stream))) {
      if (capacity < ret.len + 1) {
        capacity = (capacity == 0) ? 1024 : capacity * 2;
        ret.data = (char *)realloc(ret.data, capacity);
        if (!ret.data)
          return (Sv){0};
      }
      ret.data[ret.len++] = c;
    }
  } else {
    if (fseek(stream, 0, SEEK_END) < 0)
      printf("Error: %s\n", strerror(ferror(stream)));

    ret.len = ftell(stream);

    if (fseek(stream, 0, SEEK_SET) < 0)
      printf("Error: %s\n", strerror(ferror(stream)));

    ret.data = (char *)realloc(ret.data, ret.len);
    fread(ret.data, ret.len, 1, stream);
  }
  return ret;
}

#endif // TABLIFY_IMPLEMENTATION
