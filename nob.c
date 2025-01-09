// THIS IS THE BUILD AUTOMATION "SCRIPT"; NOT PART OF THE WEBCC COMPILER

// To build on Linux/BSD/macOS:
//   cc nob.c -o nob
//   cc -Wall -O0 nob.c -o nob
// To build on Windows:
//   TODO

static const char* common_units[] = {
  "hashmap",
  "parse",
  "preprocess",
  "stb_sprintf",
  "strings",
  "tokenize",
  "type",
  "unicode",
  "codegen",
};

static const char* wasm_units[] = {
  // XXX split? unittest*.wasm needs libc_anypct_wasm32 but not wasm_lib
  "wasm_lib",
  "libc_anypct_wasm32",
};

static const char* cli_units[] = {
  "cli_main",
};

static const char* unittest_units[] = {
  "unittest",
};

#ifndef CLANG_BIN
#define CLANG_BIN "clang"
#endif

#ifndef WASM_LD_BIN
#define WASM_LD_BIN "wasm-ld"
#endif

#ifndef SYSTEM_CC
#define SYSTEM_CC "cc" // TODO windows?
#endif

#ifndef OPT
#define OPT "2"
#endif

#define EMIT_CFGS \
X(CLANG_BIN) \
X(WASM_LD_BIN) \
X(SYSTEM_CC) \
X(OPT)

#define NOB_IMPLEMENTATION
#include "nob.h"

char* prg;

static void print_webcc_targets(FILE* out)
{
  fprintf(out, "webcc (C-to-WASM compiler) targets:\n");
  fprintf(out, "   webcc               native binary, built with system compiler (" SYSTEM_CC ")\n");
  fprintf(out, "   webcc0w.wasm        built with clang --target=wasm32\n");
  fprintf(out, "   webcc<N>w.wasm      built with webcc<N-1>w.wasm (for 1<=N<=9)\n");
  fprintf(out, "   webcc1n.wasm        built with webcc\n");
  fprintf(out, "   webcc<N>n.wasm      built with webcc<N-1>n.wasm (for 2<=N<=9)\n");
}

static void usage(const char* error)
{
  FILE* out = error != NULL ? stderr : stdout;
  if (error != NULL) fprintf(out, "%s\n\n", error);

  fprintf(out, "Usage:\n");
  fprintf(out, "   %s build <target>\n", prg);
  fprintf(out, "   %s run <target> [args]...\n", prg);
  fprintf(out, "   %s set <key> <value> [key] [value]...\n", prg);
  fprintf(out, "   %s export_config\n", prg);
  fprintf(out, "\n");

  fprintf(out, "Config:\n");
  #define X(x) fprintf(out, "  %s=\"%s\"\n", #x, x);
  EMIT_CFGS
  #undef X
  fprintf(out, "(change with \"set\"-command)\n\n");

  print_webcc_targets(out);
  fprintf(out, "\n");

  fprintf(out, "test targets:\n");
  fprintf(out, "   unittest            unittest.c native build\n");
  fprintf(out, "   unittest.<CC>.wasm  built with <CC>\n");
  fprintf(out, "   test.<T>            test/<T>.c native build\n");
  fprintf(out, "   test.<T>.<CC>.wasm  built with <CC>\n");
  fprintf(out, "<CC> is a webcc target name, or \"clang\" for clang --target=wasm32 build\n\n");

  exit(error != NULL ? EXIT_FAILURE : EXIT_SUCCESS);
  assert(!"unreachable");
}

static void bad_webcc_target(const char* error)
{
  if (error != NULL) fprintf(stderr, "%s\n\n", error);
  print_webcc_targets(stderr);
  exit(EXIT_FAILURE);
  assert(!"unreachable");
}

enum compiler_type {
  //CC,
  //CLANG,
  WEBCC_NATIVE,
  WEBCC_Xw,
  WEBCC_Xn,
};

static enum compiler_type webcc_target_type;
static int webcc_target_x;

//static enum compiler_type webcc_builder_type;
//static int webcc_builder_x;

static int read_webcc_target(char* t, size_t tn)
{
  const char* webcc = "webcc";
  const size_t webccn = strlen(webcc);
  const char* dotwasm = ".wasm";
  const size_t dotwasmn = strlen(dotwasm);
  if ((tn < webccn) || (0!=memcmp(t,webcc,webccn))) return 0;
  if (tn == webccn) {
    webcc_target_type = WEBCC_NATIVE;
    //webcc_builder_type = CLANG;
    return 1;
  } else if (tn == (webccn+2+dotwasmn) && 0==memcmp(t+webccn+2,dotwasm,dotwasmn)) {
    const char xc = t[webccn];
    const char yc = t[webccn+1];
    if (!('0' <= xc && xc <= '9')) bad_webcc_target("invalid webccXY.wasm target: X not a digit");
    const int x = xc-'0';
    if (yc == 'w') {
      webcc_target_type = WEBCC_Xw;
      webcc_target_x = x;
      if (x == 0) {
        //webcc_builder_type = CLANG;
      } else {
        //webcc_builder_type = WEBCC_Xw;
        //webcc_builder_x = x-1;
      }
      return 1;
    } else if (yc == 'n') {
      if (x == 0) bad_webcc_target("invalid webccXn.wasm target: X must be at least 1");
      webcc_target_type = WEBCC_Xn;
      webcc_target_x = x;
      if (x == 1) {
        //webcc_builder_type = WEBCC_NATIVE;
      } else {
        assert(x>=2);
        //webcc_builder_type = WEBCC_Xn;
        //webcc_builder_x = x-1;
      }
      return 1;
    } else {
      bad_webcc_target("invalid webccXY.wasm target: Y must be in [wn]");
    }
  } else {
    bad_webcc_target("invalid webcc* target");
  }
  assert(!"unreachable");
}

Nob_Procs procs;

static void do_cmd(Nob_Cmd cmd)
{
  Nob_Proc p = nob_cmd_run_async(cmd);
  nob_da_append(&procs, p);
}

static void do_join(void)
{
  nob_procs_wait_and_reset(&procs);
}

Nob_Cmd cflags;
Nob_Cmd objs;

static void reset(void)
{
  cflags.count = 0;
  objs.count = 0;
}

static void push_cdef(const char* def)
{
  // TODO windows?
  nob_cmd_append(&cflags, nob_temp_sprintf("-D%s", def));
}

static void cmd_opt(Nob_Cmd* cmd)
{
  const char* o = OPT;
  if (strcmp("g",o)==0) {
    nob_cmd_append(cmd, "-O0", "-g");
  } else if (strcmp("s",o)==0) {
    nob_cmd_append(cmd, "-Os");
  } else if (strcmp("0",o)==0) {
    nob_cmd_append(cmd, "-O0");
  } else if (strcmp("1",o)==0) {
    nob_cmd_append(cmd, "-O1");
  } else if (strcmp("2",o)==0) {
    nob_cmd_append(cmd, "-O2");
  } else {
    nob_log(NOB_WARNING, "unhandled OPT=%s", o);
  }
}

char* unit2c(const char* unit)
{
  return nob_temp_sprintf("%s.c", unit);
}

char* unit2o(const char* unit)
{
  return nob_temp_sprintf("%s.o", unit);
}

static void build_system_cc_unit(const char* unit)
{
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, SYSTEM_CC);
  nob_cmd_append(&cmd, "-std=c11", "-Wall");
  cmd_opt(&cmd);
  nob_cmd_extend(&cmd, &cflags);
  nob_cmd_append(&cmd, "-c", unit2c(unit));
  do_cmd(cmd);
  nob_cmd_append(&objs, unit2o(unit));
}

static void build_clang_wasm32_unit(const char* unit)
{
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, CLANG_BIN);
  nob_cmd_append(&cmd, "-std=c11", "-Wall");
  cmd_opt(&cmd);
  nob_cmd_append(&cmd, "--target=wasm32", "-mbulk-memory", "-nostdlib");
  nob_cmd_extend(&cmd, &cflags);
  nob_cmd_append(&cmd, "-c", unit2c(unit));
  do_cmd(cmd);
  nob_cmd_append(&objs, unit2o(unit));
}

static void build_webcc_unit(enum compiler_type target_type, int target_x, const char* unit)
{
  switch (target_type) {
  case WEBCC_NATIVE:
    build_system_cc_unit(unit);
    break;
  case WEBCC_Xw:
    if (target_x == 0) {
      build_clang_wasm32_unit(unit);
    } else {
      assert(!"TODO");
    }
    break;
  case WEBCC_Xn:
    assert(!"TODO");
    break;
  default: assert(!"unreachable");
  }
}

static void link_native(const char* artifact)
{
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, SYSTEM_CC);
  nob_cmd_extend(&cmd, &objs);
  nob_cmd_append(&cmd, "-o", artifact);
  if (!nob_cmd_run_sync(cmd)) exit(EXIT_FAILURE);
}

static void link_wasm(const char* artifact)
{
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, WASM_LD_BIN, "--no-entry", "--import-memory", "--export-dynamic");
  nob_cmd_extend(&cmd, &objs);
  nob_cmd_append(&cmd, "-o", artifact);
  if (!nob_cmd_run_sync(cmd)) exit(EXIT_FAILURE);
}

static char* webcc_wasm_artifact(enum compiler_type target_type, int target_x)
{
  assert(0<=target_x && target_x<=9);
  switch (target_type) {
  case WEBCC_NATIVE: return nob_temp_sprintf("webcc");
  case WEBCC_Xw: return nob_temp_sprintf("webcc%dw.wasm", target_x);
  case WEBCC_Xn: return nob_temp_sprintf("webcc%dh.wasm", target_x);
  default: assert(!"unreachable");
  }
  assert(!"unreachable");
}

static void artifact_stat(const char* artifact)
{
  // TODO not on windows?
  // TODO configure option that supresses this?
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, "gzip", "-fk9", artifact);
  if (!nob_cmd_run_sync(cmd)) exit(EXIT_FAILURE);
  cmd.count = 0;
  nob_cmd_append(&cmd, "ls", "-l", artifact, nob_temp_sprintf("%s.gz", artifact));
  if (!nob_cmd_run_sync(cmd)) exit(EXIT_FAILURE);
}

static void use_libc(void)
{
  push_cdef("USE_LIBC");
}

static void use_libc_anypct_wasm32(void)
{
  push_cdef("USE_LIBC_ANYPCT_WASM32");
}

static char* build_webcc(enum compiler_type target_type, int target_x)
{
  #define HANDLE_UNITS(xs) for(size_t i=0;i<NOB_ARRAY_LEN(xs);i++) build_webcc_unit(target_type, target_x, (xs)[i]);
  reset();
  switch (target_type) {
  case WEBCC_NATIVE:
    use_libc();
    HANDLE_UNITS(cli_units)
    break;
  case WEBCC_Xw:
  case WEBCC_Xn:
    use_libc_anypct_wasm32();
    HANDLE_UNITS(wasm_units)
    break;
  default: assert(!"unreachable");
  }
  HANDLE_UNITS(common_units)
  #undef HANDLE_UNITS
  do_join();

  char* artifact;
  switch (target_type) {
  case WEBCC_NATIVE:
    artifact = "webcc";
    link_native(artifact); // XXX webcc.exe on windows?
    break;
  case WEBCC_Xw:
  case WEBCC_Xn:
    artifact = webcc_wasm_artifact(target_type, target_x);
    link_wasm(artifact);
    break;
  default: assert(!"unreachable");
  }

  artifact_stat(artifact);
  return artifact;
}

static int handle_unittest_target(char* t, size_t tn)
{
  const char* unittest = "unittest";
  const size_t unittestn = strlen(unittest);
  if (tn < unittestn || memcmp(t,unittest,unittestn) != 0) return 0;
  if (strcmp("unittest", t) == 0) {
    reset();
    use_libc();
    #define HANDLE_UNITS(xs) for(size_t i=0;i<NOB_ARRAY_LEN(xs);++i) build_system_cc_unit(xs[i]);
    HANDLE_UNITS(common_units)
    HANDLE_UNITS(unittest_units)
    #undef HANDLE_UNITS
    do_join();
    link_native(t);
    return 1;
  } else {
    assert(!"TODO"); // TODO
  }
  assert(!"unreachable");
}


int main(int argc, char **argv)
{
  #define X(x) nob_go_define(#x, x);
  EMIT_CFGS
  #undef X
  NOB_GO_REBUILD_URSELF(argc, argv);

  prg = argv[0];

  if (argc < 2) usage("");

  const int is_build = (strcmp("build", argv[1]) == 0);
  const int is_run   = (strcmp("run",   argv[1]) == 0);

  if (is_build || is_run) {
    if (argc < 3) usage("missing target");
    if (is_build && argc > 3) usage("too many args");
    char* t = argv[2];
    const size_t tn = strlen(t);
    char* artifact = NULL;
    if (read_webcc_target(t, tn)) {
      artifact = build_webcc(webcc_target_type, webcc_target_x);
      exit(EXIT_SUCCESS);
    } else if (handle_unittest_target(t,tn)) {
      artifact = t;
    } else {
      fprintf(stderr, "unhandled non-webcc target \"%s\" (TODO tests?)\n", t);
      exit(EXIT_FAILURE);
    }

    if (is_run) {
      switch (webcc_target_type) {
      case WEBCC_NATIVE: {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, nob_temp_sprintf("./%s", artifact));
        for (int i=3; i < argc; i++) nob_cmd_append(&cmd, argv[i]);
        if (!nob_cmd_run_sync(cmd)) exit(EXIT_FAILURE);
      } break;
      case WEBCC_Xw:
      case WEBCC_Xn:
        assert(!"TODO");
        break;
      default: assert(!"unreachable");
      }
    }

    exit(EXIT_SUCCESS);
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
    exit(EXIT_SUCCESS);
  } else if (strcmp("export_config", argv[1]) == 0) {
    // XXX doesn't properly escape if necessary... maybe steal
    // nob__write_escaped_string()?
    printf("%s set", prg);
    puts(
    #define X(x) " " #x " " x
    EMIT_CFGS
    #undef X
    "\n"
    );
    exit(EXIT_SUCCESS);
  } else {
    usage("invalid command");
  }
  assert(!"unreachable");
}
