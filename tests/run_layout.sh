#!/bin/sh
set -eu
test_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_build=${BTREE_TEST_BUILD:-"$test_root/.scratch/node-layout"}
test_include=${BTREE_TEST_INCLUDE_ROOT:-"$test_root"}
mkdir -p "$test_build"
test_compiler=${CXX:-c++}
for standard in 17 20; do
    for mode in release debug; do
        case "$mode" in release) diagnostic=-DNDEBUG ;; debug) diagnostic=-UNDEBUG ;; esac
        output="$test_build/cxx$standard-$mode"
        if [ "${BTREE_TEST_SANITIZE:-0}" = 1 ]; then
            "$test_compiler" -std=c++"$standard" -O1 -g -fsanitize=address,undefined,object-size,pointer-overflow -fno-sanitize-recover=all -fno-omit-frame-pointer "$diagnostic" -I "$test_include" "$test_root/tests/node_layout.cc" -o "$output"
        else
            "$test_compiler" -std=c++"$standard" -O2 "$diagnostic" -I "$test_include" "$test_root/tests/node_layout.cc" -o "$output"
        fi
        if [ -n "${BTREE_TEST_CASE:-}" ]; then
            "$output" "$BTREE_TEST_CASE"
        else
            "$output"
        fi
        printf 'PASS compact layout C++%s %s\n' "$standard" "$mode"
    done
done
