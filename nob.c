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
   for (int i=1; i<argc; i++) {
      char* arg = argv[i];
	  #define CFG(x) if (strlen(argv[i]) >= (1+strlen(#x)) && 0 == memcmp(arg, #x "=", (1+strlen(#x)))) NOB_GO_REDEFINE_STR(#x, arg+(1+strlen(#x)));
	  CFG(CLANG_BIN)
	  CFG(WASM_LD_BIN)
	  #undef CFG
   }
   DD(CLANG_BIN);
   DD(WASM_LD_BIN);
}
