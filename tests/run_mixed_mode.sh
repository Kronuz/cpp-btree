#!/bin/sh
set -eu
test_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_build="$test_root/.scratch/mixed-mode"
mkdir -p "$test_build"
test_compiler=${CXX:-c++}
for standard in 17 20; do
    for modes in RR RD DR DD; do
        case "$modes" in R*) main_mode=-DNDEBUG ;; *) main_mode=-UNDEBUG ;; esac
        case "$modes" in *R) mutate_mode=-DNDEBUG ;; *) mutate_mode=-UNDEBUG ;; esac
        stem="$test_build/cxx$standard-$modes"
        if [ "${BTREE_TEST_SANITIZE:-0}" = 1 ]; then
            "$test_compiler" -std=c++"$standard" -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer "$main_mode" -I "$test_root" -c "$test_root/tests/mixed_mode_main.cc" -o "$stem-main.o"
            "$test_compiler" -std=c++"$standard" -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer "$mutate_mode" -I "$test_root" -c "$test_root/tests/mixed_mode_mutate.cc" -o "$stem-mutate.o"
        else
            "$test_compiler" -std=c++"$standard" -O2 "$main_mode" -I "$test_root" -c "$test_root/tests/mixed_mode_main.cc" -o "$stem-main.o"
            "$test_compiler" -std=c++"$standard" -O2 "$mutate_mode" -I "$test_root" -c "$test_root/tests/mixed_mode_mutate.cc" -o "$stem-mutate.o"
        fi
        for order in main-first mutate-first; do
            case "$order" in main-first) first=main; second=mutate ;; *) first=mutate; second=main ;; esac
            if [ "${BTREE_TEST_SANITIZE:-0}" = 1 ]; then
                "$test_compiler" -fsanitize=address,undefined "$stem-$first.o" "$stem-$second.o" -o "$stem-$order"
            else
                "$test_compiler" "$stem-$first.o" "$stem-$second.o" -o "$stem-$order"
            fi
            "$stem-$order"
            printf 'PASS C++%s %s %s\n' "$standard" "$modes" "$order"
        done
    done
done
