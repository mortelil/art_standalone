#!/bin/sh
# SPDX-License-Identifier: Apache-2.0
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
prefix=${ATL_PREFIX:-$(dirname "$repo")/.atl-runtime}
out="$repo/mobile-test-output"
mkdir -p "$out"
export LD_LIBRARY_PATH="$prefix/lib:/usr/lib/art${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
g++ -std=c++17 -DHAVE_SYS_UIO_H \
	-I "$repo/system/core/libutils/include" -I "$repo/system/core/libcutils/include" \
	-I "$repo/libbase/include" -I "$repo/system/core/include" \
	"$repo/tests/mobile/test-looper.cpp" -L "$prefix/lib" -L /usr/lib/art \
	-lutils -lcutils -llog -o "$out/test-looper"
timeout 30 "$out/test-looper"
g++ -std=c++17 -I "$repo" "$repo/tests/mobile/test-signal-stack.cpp" -o "$out/test-signal-stack"
timeout 30 "$out/test-signal-stack"
