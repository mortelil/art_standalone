#!/bin/sh
set -eu
mkdir -p "$1"
cd "$1"
for name in rsa other ca; do
  ca=FALSE; test "$name" != ca || ca=TRUE
  openssl req -x509 -newkey rsa:2048 -nodes -keyout "$name.key" -out "$name.pem" -days 2 -subj /CN=fixture -addext "basicConstraints=critical,CA:$ca" 2>/dev/null
done
openssl req -x509 -newkey ec -pkeyopt ec_paramgen_curve:P-256 -nodes -keyout ec.key -out ec.pem -days 2 -subj /CN=ec-fixture -addext basicConstraints=critical,CA:FALSE 2>/dev/null
openssl req -new -newkey rsa:2048 -nodes -keyout leaf.key -out leaf.csr -subj /CN=leaf 2>/dev/null
openssl x509 -req -in leaf.csr -CA ca.pem -CAkey ca.key -CAcreateserial -out leaf.pem -days 2 2>/dev/null
