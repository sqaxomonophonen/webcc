#ifndef WEBCC_H

int verrorf(const char* fmt, va_list ap);
int errorf(const char* fmt, ...);
void* scratch_calloc(size_t number, size_t size);
void* scratch_realloc(void* ptr, size_t size);

#define PLEASE_EXPORT __attribute__((visibility("default")))

#define WEBCC_H
#endif
