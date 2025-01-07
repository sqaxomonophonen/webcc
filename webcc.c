#include "chibicc.h"

char *base_file; // XXX?

PLEASE_EXPORT void webcc0(void)
{
	init_macros();
}

PLEASE_EXPORT void webcc(char* fn)
{
  // TODO XXX
  Token* tok = tokenize_file(fn);
  tok = preprocess(tok);
  Obj *prog = parse(tok);
  codegen(prog, NULL, 0); // FIXME provide buffer
}

void* scratch_calloc(size_t number, size_t size)
{
  assert(!"TODO");
}

void* scratch_realloc(void* ptr, size_t size)
{
  assert(!"TODO");
}

int verrorf(const char* fmt, va_list ap)
{
  assert(!"TODO");
}

int errorf(const char* fmt, ...)
{
  assert(!"TODO");
}
