#!/usr/bin/env bash
set -e
for target in hashmap codegen tokenize ; do
  echo -n "$target: "
  cc -DUSE_LIBC -DUNIT_TEST ${target}.c -o test_${target} && ./test_${target} && rm ./test_${target}
done
