#include "chibicc.h"
void anypct_handle_failed_assertion(const char* failed_predicate, const char* location)
{
  abort();
}

size_t strlen(const char *s)
{
  assert(!"TODO");
}

int memcmp(const void* b1, const void* b2, size_t len)
{
  assert(!"TODO");
}

int strcmp(const char* s1, const char* s2)
{
  assert(!"TODO");
}

char* strndup(const char *str, size_t len)
{
  assert(!"TODO");
}

int strncmp(const char* s1, const char* s2, size_t len)
{
  assert(!"TODO");
}

char* strncpy(char* restrict dst, const char* restrict src, size_t len)
{
  assert(!"TODO");
}

int strncasecmp(const char* s1, const char* s2, size_t len)
{
  assert(!"TODO");
}

int isxdigit(int c)
{
  assert(!"TODO");
}

int isdigit(int c)
{
  assert(!"TODO");
}

int isspace(int c)
{
  assert(!"TODO");
}

int isalnum(int c)
{
  assert(!"TODO");
}

int ispunct(int c)
{
  assert(!"TODO");
}

unsigned long strtoul(const char* restrict nptr, char** restrict endptr, int base)
{
  assert(!"TODO");
}

double strtod(const char* restrict nptr, char** restrict endptr)
{
  assert(!"TODO");
}

char* strstr(const char* big, const char* little)
{
  assert(!"TODO");
}

char* strchr(const char* s, int c)
{
  assert(!"TODO");
}


// XXX below this line are functions that are not directly referenced, but that
// may be referenced through optimizations.

void* memchr(const void* b, int c, size_t len)
{
  assert(!"TODO");
}
