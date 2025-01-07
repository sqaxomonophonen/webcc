#include "chibicc.h"

void anypct_handle_failed_assertion(const char* failed_predicate, const char* location)
{
  abort();
}

int isdigit(int c)
{
  return '0'<=c && c<= '9';
}

int isxdigit(int c)
{
  return isdigit(c) || ('a'<=c && c<='f') || ('A'<=c && c<='F');
}

int isspace(int c)
{
  switch (c) {
  case '\t':
  case '\n':
  case '\v':
  case '\f':
  case '\r':
  case ' ':
    return 1;
  default:
    return 0;
  }
  assert(!"unreachable");
}

int isalpha(int c)
{
  return ('a'<=c && c<='z') || ('A'<=c && c<='Z');
}

int isalnum(int c)
{
  return isdigit(c) || isalpha(c);
}

int ispunct(int c)
{
  switch (c) {
  case '!': case '"': case '#': case '$': case '%':
  case '&': case '\'':case '(': case ')': case '*':
  case '+': case ',': case '-': case '.': case '/':
  case ':': case ';': case '<': case '=': case '>':
  case '?': case '@': case '[': case '\\':case ']':
  case '^': case '_': case '`': case '{': case '|':
  case '}': case '~':
    return 1;
  default:
    return 0;
  }
  assert(!"unreachable");
}

size_t strlen(const char *s)
{
  size_t n=0;
  for (const char* p=s; *p; p++, n++) {}
  return n;
}

int memcmp(const void* b1, const void* b2, size_t len)
{
  const unsigned char* p1 = b1;
  const unsigned char* p2 = b2;
  for (size_t i=0; i<len; i++, p1++, p2++) {
    const int c1 = *p1;
    const int c2 = *p2;
    const int d = c1-c2;
    if (d!=0) return d;
  }
  return 0;
}

int strcmp(const char* s1, const char* s2)
{
  assert(!"TODO");
}

int strncmp(const char* s1, const char* s2, size_t len)
{
  assert(!"TODO");
}

int strncasecmp(const char* s1, const char* s2, size_t len)
{
  assert(!"TODO");
}

char* strchr(const char* s, int c)
{
  for (const char* p=s; *p; p++) if (*p == c) return (char*)p;
  return NULL;
}

void* memchr(const void* b, int c, size_t len)
{
  for (size_t i=0; i<len; i++) if (((unsigned char*)b)[i]==c) return (void*)b+i;
  return NULL;
}

char* strndup(const char *str, size_t len)
{
  assert(!"TODO");
}

char* strncpy(char* restrict dst, const char* restrict src, size_t len)
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
