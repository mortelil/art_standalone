#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Run in Alpine; CORE_JAR and RUNTIME_LIB identify the selected ART runtime.
set -eu
repo=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
sh "$repo/tests/trust-anchors/fixtures.sh" "$out/fixtures"
mkdir -p "$out/classes" "$out/probe" "$out/cache"
export XDG_CACHE_HOME="$out/cache"
find "$repo/external/wolfssljni/src/java" -name '*.java' ! -name WolfSSLJDK8Helper.java > "$out/sources"
jdk=${JAVA_HOME:-/usr/lib/jvm/java-1.8-openjdk}
"$jdk/bin/javac" -source 8 -target 8 -d "$out/classes" @"$out/sources"
"$jdk/bin/jar" cf "$out/wolf.jar" -C "$out/classes" .
dx --dex --output="$out/wolf-dex.jar" "$out/wolf.jar"
"$jdk/bin/javac" -d "$out/probe" "$repo/tests/trust-anchors/TrustAnchors.java"
"$jdk/bin/java" -cp "$out/probe" TrustAnchors "$out/fixtures" platform
dx --dex --output="$out/probe.jar" "$out/probe"
boot=${CORE_JAR:?Set CORE_JAR}
for name in apachehttp apache-xml bouncycastle core-junit hamcrest junit-runner okhttp; do
 boot=$boot:/usr/lib/java/dex/art/$name-hostdex.jar
done
export LD_LIBRARY_PATH="${RUNTIME_LIB:?Set RUNTIME_LIB}:/usr/lib/art:/usr/lib/java/dex/art/natives:/usr/lib"
ulimit -c 0
sha256sum "$out/wolf-dex.jar" "$CORE_JAR"
timeout 60 dalvikvm -Xbootclasspath:"$boot:$out/wolf-dex.jar" -cp "$out/probe.jar" TrustAnchors "$out/fixtures" wolfJSSE
