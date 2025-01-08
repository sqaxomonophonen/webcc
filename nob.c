// THIS IS THE BUILD AUTOMATION "SCRIPT"; NOT PART OF THE WEBCC COMPILER

// To build on Linux/BSD/macOS:
//   cc nob.c -o nob
//   cc -Wall -O0 nob.c -o nob
// To build on Windows:
//   TODO

#define NOB_IMPLEMENTATION
#include "nob.h"

char* prg;
static void usage(const char* error)
{
  FILE* out = error != NULL ? stderr : stdout;
  if (error != NULL) fprintf(out, "%s\n", error);
  fprintf(out, "Usage:\n");
  fprintf(out, "   %s config [...]\n", prg);
  fprintf(out, "   %s <cmd>\n", prg);
  fprintf(out, "   %s <cmd>\n", prg);
  exit(error != NULL ? EXIT_FAILURE : EXIT_SUCCESS);
}

#define DD(x) printf("%s=[%s]\n",#x,x)

#ifndef CLANG_BIN
#define CLANG_BIN "clang"
#endif

#ifndef WASM_LD_BIN
#define WASM_LD_BIN "wasm-ld"
#endif

int main(int argc, char **argv)
{
   NOB_GO_DEFINE(CLANG_BIN); 
   NOB_GO_DEFINE(WASM_LD_BIN);
   NOB_GO_REBUILD_URSELF(argc, argv);
   #if 0
   for (int i=1; i<argc; i++) {
      char* arg = argv[i];
      if (strlen(argv[i]) >= 4 && 0 == memcmp(arg, "FOO=", 4)) NOB_GO_REDEFINE(FOO, arg+4);
      if (strlen(argv[i]) >= 4 && 0 == memcmp(arg, "BAR=", 4)) NOB_GO_REDEFINE(BAR, arg+4);
   }
   DD(FOO);
   DD(BAR);
   #endif
}
