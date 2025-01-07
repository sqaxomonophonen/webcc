#include "chibicc.h"

void strarray_push(StringArray *arr, char *s) {
  if (!arr->data) {
    arr->data = scratch_calloc(8, sizeof(char *));
    arr->capacity = 8;
  }

  if (arr->capacity == arr->len) {
    arr->data = scratch_realloc(arr->data, sizeof(char *) * arr->capacity * 2);
    arr->capacity *= 2;
    for (int i = arr->len; i < arr->capacity; i++)
      arr->data[i] = NULL;
  }

  arr->data[arr->len++] = s;
}

// Takes a printf-style format string and returns a formatted string.
char *format(char *fmt, ...) {
  char buf[1<<16];
  va_list ap;
  va_start(ap, fmt);
  int n = stbsp_vsnprintf(buf, sizeof buf, fmt, ap);
  va_end(ap);
  char* s = scratch_calloc(n+1,1);
  memcpy(s,buf,n);
  s[n]=0;
  return s;
}
