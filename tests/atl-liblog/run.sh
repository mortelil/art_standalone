#!/bin/sh
set -eu
src=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
lib=$(realpath "$1")
out=$(mktemp -d)
trap 'rm -f "$out/redirect"; rmdir "$out"' EXIT
cc "$src/redirect.c" -ldl -o "$out/redirect"
LIBLOG_COLOR=never timeout 10 "$out/redirect" "$lib"
