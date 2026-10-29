#!/bin/sh
# Runs the library's host tests on a computer (no board needed): ArduinoBLE is simulated.
# Usage: sh extras/host-tests/run.sh      (needs clang++ or g++)
set -e
here=$(cd "$(dirname "$0")" && pwd)
src="$here/../../src"
out=$(mktemp -d)
for test in "$here"/*.cpp; do
  name=$(basename "$test" .cpp)
  ${CXX:-clang++} -std=c++17 -g -fsanitize=address,undefined -I"$here/mock" -I"$src" "$test" "$src/DFPongController.cpp" -o "$out/$name"
  echo "== $name"
  "$out/$name" | grep -E "^ok|^FAIL|failure"
done
