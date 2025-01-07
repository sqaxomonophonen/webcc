#ifndef LIBC_ANYPCT_WASM32_H

#ifndef __wasm32__
#error "not wasm32"
// one reason this is not usable outside wasm32: it uses __builtin_memcpy for
// memmove because it maps to wasm32 memory.copy which has memmove semantics
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define noreturn _Noreturn

noreturn void anypct_handle_failed_assertion(const char* failed_predicate, const char* location);
#define ANYPCT_STR2(s) #s
#define ANYPCT_STR(s) ANYPCT_STR2(s)

#define assert(p) if (!(p)) { anypct_handle_failed_assertion(#p, __FILE__ ":" ANYPCT_STR(__LINE__)); }

#define memcpy  __builtin_memcpy // -> memory.copy
#define memmove __builtin_memcpy // -> memory.copy (has memmove semantics)
#define memset  __builtin_memset // -> memory.fill

#define abort __builtin_trap

int isspace(int c);
int ispunct(int c);
int isalpha(int c);
int isalnum(int c);
int isdigit(int c);
int isxdigit(int c);

unsigned long strtoul(const char* nptr, char** endptr, int base);
double strtod(const char* restrict nptr, char** restrict endptr);

size_t strlen(const char* s);
int memcmp(const void* b1, const void* b2, size_t len);
int strcmp(const char* s1, const char* s2);
char* strndup(const char *str, size_t len);
int strncmp(const char* s1, const char* s2, size_t len);
char* strncpy(char* restrict dst, const char* restrict src, size_t len);
int strncasecmp(const char* s1, const char* s2, size_t len);
char* strstr(const char* big, const char* little);
char* strchr(const char* s, int c);

// XXX below this line are functions that are not directly referenced, but that
// may be referenced through optimizations.

void* memchr(const void* b, int c, size_t len);

#define LIBC_ANYPCT_WASM32_H
#endif
