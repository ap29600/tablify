#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifndef STRINGVIEW_H_
#define STRINGVIEW_H_

#define SV_FMT "%.*s"
#define SV_ARG(sv) (int)(sv).len, (sv).data
#define MIN(a, b) (a) < (b) ? (a) : (b)

typedef struct {
  size_t len;
  char *data;
} string_view;

#define SV_CONST(string)                                                       \
  (string_view) { .len = strlen((string)), .data = (string) }

string_view sv_str(char *data);
string_view sv_nstr(char *data, size_t len);
string_view sv_trim(string_view sv);
string_view sv_split(string_view *sv, char delim);
int sv_cmp(string_view a, string_view b);
int sv_starts_with(string_view a, char* b);

#endif // STRINGVIEW_H_

#ifdef STRINGVIEW_IMPLEMENTATION

string_view sv_str(char *data) {
  return (string_view){.len = strlen(data), .data = data};
}

string_view sv_nstr(char *data, size_t len) {
  return (string_view){.len = MIN(strlen(data), len), .data = data};
}

string_view sv_trim(string_view sv) {
  while (isspace(sv.data[0]) && sv.len > 0) {
    sv.data++;
    sv.len--;
  }
  while (isspace(sv.data[sv.len - 1]) && sv.len > 0)
    sv.len--;
  return sv;
}

string_view sv_split(string_view *sv, char delim) {
  char *data = sv->data;
  size_t len;
  while (sv->data[0] != delim && sv->len > 0) {
    sv->data++;
    sv->len--;
  }
  if (sv->len > 0) { // get rid of the delimiter
    sv->data++;
    sv->len--;
    len = sv->data - data - 1;
  } else {
    len = sv->data - data;
  }
  return (string_view){.len = len, .data = data};
}

int sv_cmp(string_view a, string_view b) {
  return a.len == b.len ? strncmp(a.data, b.data, a.len) : 1;
}

int sv_starts_with(string_view a, char *b) {
  return !strncmp(a.data, b, strlen(b));
}

#undef STRINGVIEW_IMPLEMENTATION
#endif // STRINGVIEW_IMPLEMENTATION
