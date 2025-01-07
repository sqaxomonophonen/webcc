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
  assert(!"TODO");
}

int errorf(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  int n = verrorf(fmt, ap);
  va_end(ap);
  return n;
}

int main(int argc, char** argv)
{
  printf("hello world (TODO)\n");
  return 0;
}
