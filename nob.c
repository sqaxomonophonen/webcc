// THIS IS THE BUILD AUTOMATION "SCRIPT"; NOT PART OF THE WEBCC COMPILER

// To build on Linux/BSD/macOS:
//   cc nob.c -o nob
//   cc -Wall -O0 nob.c -o nob
// To build on Windows:
//   TODO

#define NOB_IMPLEMENTATION
#include "nob.h"

#ifndef CLANG_BIN
#define CLANG_BIN "clang"
#endif

#ifndef WASM_LD_BIN
#define WASM_LD_BIN "wasm-ld"
#endif

#define EMIT_CFGS \
  X(CLANG_BIN) \
  X(WASM_LD_BIN)

char* prg;
static void usage(const char* error)
{
  FILE* out = error != NULL ? stderr : stdout;
  if (error != NULL) fprintf(out, "%s\n", error);
  fprintf(out, "Usage:\n");
  fprintf(out, "   %s build <target>\n", prg);
  fprintf(out, "   %s run <target> [args]...\n", prg);
  fprintf(out, "   %s set <key> <value> [key] [value]...\n", prg);

  fprintf(out, "\nConfig:\n");
  #define X(x) fprintf(out, "  %s=\"%s\"\n", #x, x);
  EMIT_CFGS
  #undef X
  fprintf(out, "(change with \"set\"-command)\n");

  fprintf(out, "\nwebcc targets:\n");
  fprintf(out, "   webcc               natively built\n");
  fprintf(out, "   webcc0w.wasm        built with clang --target=wasm32\n");
  fprintf(out, "   webcc<N>w.wasm      built with webcc<N-1>w.wasm (for N>=1)\n");
  fprintf(out, "   webcc1n.wasm        built with webcc\n");
  fprintf(out, "   webcc<N>n.wasm      built with webcc<N-1>n.wasm (for N>=2)\n");

  fprintf(out, "\ntest targets:\n");
  fprintf(out, "   unittest            unittest.c native build\n");
  fprintf(out, "   unittest.<CC>.wasm  built with <CC>\n");
  fprintf(out, "   test.<T>            test/<T>.c native build\n");
  fprintf(out, "   test.<T>.<CC>.wasm  built with <CC>\n");
  fprintf(out, "<CC> can be one of the webcc target names, or \"clang\"\n\n");

  exit(error != NULL ? EXIT_FAILURE : EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
  #define X(x) nob_go_define(#x, x);
  EMIT_CFGS
  #undef X
  NOB_GO_REBUILD_URSELF(argc, argv);

  prg = argv[0];

  if (argc < 2) usage("");

  if (strcmp("build", argv[1]) == 0) {
    assert(!"TODO");
  } else if (strcmp("run", argv[1]) == 0) {
    assert(!"TODO");
  } else if (strcmp("set", argv[1]) == 0) {
    if ((argc % 2) != 0) {
        fprintf(stderr, "odd number of \"set\"-arguments; must come in key/value pairs\n");
        exit(EXIT_FAILURE);
    }
    for (int i=2; i<(argc-1); i+=2) {
      char* arg = argv[i];
      char* val = argv[i+1];
      int match = 0;
      #define X(x) if (strcmp(arg,#x)==0) { match=1; NOB_GO_REDEFINE_STR(#x, val); }
      EMIT_CFGS
      #undef X
      if (!match) {
        fprintf(stderr, "no such config key \"%s\"\n", arg);
        exit(EXIT_FAILURE);
      }
    }
  } else {
    usage("invalid command");
  }
  return EXIT_SUCCESS;
}
