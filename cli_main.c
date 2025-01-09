// main() for a native CLI webcc build; this is mostly to reap benefits from
// tools like gdb/gf2 and valgrind

#include "chibicc.h"

char *base_file; // XXX?

void* scratch_calloc(size_t number, size_t size)
{
  return calloc(number, size);
}

void* scratch_realloc(void* ptr, size_t size)
{
  return realloc(ptr, size);
}

int verrorf(const char* fmt, va_list ap)
{
  return vfprintf(stderr, fmt, ap);
}

int main(int argc, char** argv)
{
  init_macros();
  Token* tok = tokenize_file("stb_sprintf.h");
  //Token* tok = tokenize_file("parse.c");
  //Token* tok = tokenize_file("normalize.h");
  tok = preprocess(tok);
  printf("OK? %p\n", tok);
  return 0;
}

const char *read_source_file(const char *path)
{
  FILE* f = fopen(path, "rb");
  if (f == NULL) {
    fprintf(stderr, "%s: not found\n", path);
    exit(EXIT_FAILURE);
  }
  assert(0 == fseek(f, 0, SEEK_END));
  const long sz = ftell(f);
  assert(0 == fseek(f, 0, SEEK_SET));
  assert(0 == ftell(f));
  const size_t cap = sz+16;
  char* src = scratch_calloc(cap, 1);
  assert(1 == fread(src, sz, 1, f));
  fclose(f);
  src = normalize_source_string(src, cap);
  return src;
}
