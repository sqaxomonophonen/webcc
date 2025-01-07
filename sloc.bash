#!/usr/bin/env bash
echo ALL:
wc -l $(git ls-files)
echo

echo WEBCC:
wc -l $(git ls-files '*.c' '*.h' | grep -vF test/ | grep -vF include/ )
