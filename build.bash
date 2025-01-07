#!/usr/bin/env bash
set -e

usage() {
  echo "Usage:"
  echo " $0 <clang|webcc|cli|unittest> ..."
  echo " $0 clang <clang-binary> <wasm-ld-binary>"
  echo " $0 webcc"
  echo " $0 cli"
  echo " $0 unittest"
  exit 1
}

common_units="hashmap parse preprocess stb_sprintf strings tokenize type unicode codegen"
wasm_units="wasm_lib libc_anypct_wasm32 $common_units"
cli_units="cli_main $common_units"

wasm_artifact="webcc.wasm"

wasm_ld_opts="--no-entry --import-memory --export-dynamic"

mode=$1
case $1 in
clang)
  artifact="$wasm_artifact"
  units="$wasm_units"
  clang_bin=$2
  if [ -z "$clang_bin" ] ; then
    echo "no clang binary specified"
    usage
    exit 1
  fi
  link=$3
  if [ -z "$link" ] ; then
    echo "no wasm-ld binary specified"
    usage
    exit 1
  fi
  link="$link $wasm_ld_opts"
  ;;
webcc)
  artifact="$wasm_artifact"
  units="$wasm_units"
  link="$link $wasm_ld_opts"
  ;;
cli)
  artifact="webcc"
  units="$cli_units"
  link="cc"
  ;;
unittest)
  OPT=${OPT:-"-O0 -g"}
  artifact="unittest"
  units="unittest $common_units"
  link="cc"
  ;;
*)
  usage
  ;;
esac

OPT=${OPT:-"-O2"}
COMMON_CFLAGS="${OPT} -std=c11 -Wall"
NATIVE_CFLAGS="-DUSE_LIBC"
WASM_CFLAGS="-DUSE_LIBC_ANYPCT_WASM32"

CC() {
  case $mode in
  clang)
    set -x
    $clang_bin \
      $COMMON_CFLAGS \
      $WASM_CFLAGS \
      --target=wasm32 \
      -mbulk-memory \
      -nostdlib \
      -c $1
    set +x
    ;;
  webcc)
    echo "TODO"
    exit 2
    ;;
  cli|unittest)
    set -x
    cc \
      $COMMON_CFLAGS \
      $NATIVE_CFLAGS \
      -Wall \
      -std=c11 \
      -c $1
    set +x
    ;;
  *)
    echo "INTERNAL ERROR"
    exit 2
    ;;
  esac
}

objs=""
for u in $units ; do
  CC ${u}.c
  objs="${objs} ${u}.o"
done

set -x
$link ${objs} -o ${artifact}
set +x
# XXX gzip size is interesting. especially for .wasm since it's indicative of
# "over the wire" size. but do we want it for all targets?
gzip -fk9 ${artifact}
ls -l ${artifact} ${artifact}.gz
