#!/bin/sh
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
lib=$(realpath "$1")
out=$2
mkdir -p "$out"
c++ -std=c++11 -pthread -I"$repo/libnativehelper/include" "$repo/tests/async-close/probe.cpp" \
  -L"$lib" -Wl,-rpath,"$lib" -lnativehelper -o "$out/probe"
LD_LIBRARY_PATH="$lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" timeout 10 "$out/probe"
