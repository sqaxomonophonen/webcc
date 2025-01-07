#!/usr/bin/env bash
set -e

usage() {
  echo "Usage:"
  echo " $0 <clang|webcc> ..."
  echo " $0 clang <clang-binary> ..."
  echo " $0 webcc ..."
  exit 1
}

mode=$1
case $1 in
clang)
  clang_bin=$2
  if [ -z "$clang_bin" ] ; then
    echo "no clang binary specified"
    usage
    exit 1
  fi
  ;;
webcc)
  ;;
*)
  usage
  ;;
esac

COMMON_CFLAGS="-DUSE_LIBC_ANYPCT_WASM32"

CC() {
  case $mode in
  clang)
    set -x
    $clang_bin \
      $COMMON_CFLAGS \
      --target=wasm32 \
      -mbulk-memory \
      -O2 \
      -Wall \
      -std=c11 \
      -nostdlib \
      -c $1
    set +x
    ;;
  webcc)
    echo "TODO"
    exit 2
    ;;
  *)
    echo "INTERNAL ERROR"
    exit 2
    ;;
  esac
}

units="libc_anypct_wasm32 hashmap parse preprocess stb_sprintf strings tokenize type unicode codegen webcc"
objs=""
for u in $units ; do
  CC ${u}.c
  objs="${objs} ${u}.o"
done

wasm-ld --no-entry --import-memory --export-dynamic ${objs} -o webcc.wasm
ls -l webcc.wasm
