# Mutable-pair relocation

**Branch:** `move-relocation`, based on `master` at `25f9494`.
**Status:** Prototype. Correct on every configuration measured below; not yet qualified for release.

## Summary

`btree::map` can corrupt itself when an allocation fails while it reshapes a node. Slots hold `std::pair<const Key, T>`, and moving a pair with a const key copies the key. With `std::string` keys longer than the small-string buffer, every split, rebalance, merge, erase and root swap allocates, and a refusal halfway through leaves entries lost or misordered, leaks memory, or double-frees on erase. Every revision since the 2019 import fails this way.

This branch stores each slot as a mutable `std::pair<Key, T>`, read through a layout-checked union as `std::pair<const Key, T>&`. Relocation becomes a real move, every node a reshape needs is allocated before the first value moves, and the incoming value is built before the tree changes. With 200-byte keys, erasing 65,536 entries makes **0** allocations instead of 72,692, and mixed insert/erase CPU drops by **30%**. Node layout and memory are unchanged.

## The change

1. Map slots are a union of `std::pair<Key, T>` and `std::pair<const Key, T>`, the technique Abseil and libc++ use for const-key pairs. Static assertions require equal size, alignment and member offsets, and every access goes through `std::launder`. The layout is implementation-defined; the assertions stop a compiler that lays the two pairs out differently. Sets are unchanged.
2. `move_value` moves the mutable pair, so a key move is a move. `params::swap` no longer swaps keys through `const_cast`.
3. `rebalance_or_split` allocates each level's sibling, and any new root, before moving a value. `internal_emplace` constructs the incoming value before reshaping. A refused insertion leaves contents, structure and live allocations unchanged.
4. The per-key copy preparation that `cbc3576` added for const-key pairs is no longer needed and is removed.

Details: [`NODE_LAYOUT.md`](NODE_LAYOUT.md), "Value slots and relocation". Full measurements: [`perf/move-relocation.md`](perf/move-relocation.md).

## Evidence

| Check | `25f9494` | This branch |
| --- | --- | --- |
| Allocation-refusal sweep on Detent's model types (4 tables × insert/erase/copy × short/200-byte names) | long-name insert and erase fail (lost entries, leaks, crashes) | 0 violations in all 24 cells |
| `tests/relocation_failure.cc` (refuse every allocation of insert; erase with the heap closed) | fails | passes |
| Detent `test_model_alloc_failure` (adds copy assignment and parse/patch/diff lifecycle) | fails | passes |
| `tests/run_layout.sh`, `tests/run_mixed_mode.sh`, Apple clang 17 | pass | 16/16, 16/16 |
| `tests/run_layout.sh`, `tests/run_mixed_mode.sh`, Linux g++ 11, native | not run | pass |
| UBSan + object-size + pointer-overflow, four fixtures, C++20 | not run | pass |

`mutation_bench` (65,536 random inserts then erases, five alternating pairs, about ±3% noise from an A/A run), process CPU:

| Key bytes | `25f9494` | This branch | Ratio |
| --- | --- | --- | --- |
| 12 | 88.2 ms | 88.7 ms | 1.006 |
| 32 | 163.1 ms | 120.7 ms | 0.740 |
| 200 | 249.5 ms | 175.4 ms | 0.703 |

With 200-byte keys, insertion makes 147,659 allocations instead of 420,230 (23.8 MB requested instead of 80.5 MB). Node allocation counts, requested bytes and peak live bytes are unchanged. In Detent's 100K-knob lifecycle benchmark this branch retains exactly the same requested bytes as `25f9494` (17.0 MB with short names) and its total CPU is within 3% of a separate `25f9494` run.

## Guarantees and limits

- Relocation cannot throw when `Key` and `T` have nonthrowing moves (strings, variants, nested `btree::map`). Types whose moves can throw keep the previous basic guarantee; there is no transaction engine.
- The union depends on implementation-defined layout, checked at compile time, with no fallback path.

## Open before merging

1. **A possible hang.** The sanitized g++ 11 run of `tests/prepared_commit.cc` (C++17, Debug) spins at 100% CPU. GCC 11's AddressSanitizer is known to loop on `DEADLYSIGNAL` under high address-space randomization, which hits other binaries on the same host. A probe comparing `25f9494` and this branch with randomization on and off is pending; until it reports, a real loop is not ruled out.
2. **Sanitizer matrix.** Linux g++ and clang with `BTREE_TEST_SANITIZE=1`, FreeBSD, and macOS AddressSanitizer (it hangs at startup on the development host, even for an empty program).
3. **GCC 16.** `tests/prepared_commit.cc` aborts at `-std=c++20 -O2` on both `25f9494` and this branch; it passes at `-O0`, `-O1` and with g++ 13. Unexplained.
4. **Timing gaps.** Debug builds and libstdc++ (where 16-byte keys exceed the 15-byte small buffer) are not measured.

## Reproduce

```sh
# Library fixtures, as AGENTS.md requires (repeat with CXX=g++ and BTREE_TEST_SANITIZE=1)
sh tests/run_layout.sh && sh tests/run_mixed_mode.sh

# Allocation counts with long keys
c++ -std=c++20 -O3 -DNDEBUG -I . benchmarks/relocation_alloc_bench.cc -o relocation_alloc_bench
./relocation_alloc_bench 200
```
