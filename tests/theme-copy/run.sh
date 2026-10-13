#!/bin/sh
set -eu
src=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo=$(CDPATH= cd -- "$src/../.." && pwd)
lib=$(realpath "$1"); framework=$(realpath "$2"); out=$(realpath "$3")
aapt package -f -M "$src/fixture/AndroidManifest.xml" -S "$src/fixture/res" -I "$framework" -F "$out/theme-fixture.apk"
cc -D_LARGEFILE64_SOURCE -Wall -Wextra -I"$repo/libandroidfw/include" "$src/probe.c" -L"$lib" -Wl,-rpath,"$lib" -landroidfw -lpng -o "$out/theme-copy"
LD_LIBRARY_PATH="$lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" timeout 15 "$out/theme-copy" "$framework" "$out/theme-fixture.apk"
