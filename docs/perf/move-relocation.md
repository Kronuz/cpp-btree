# Mutable-pair relocation

Question: can a narrow change to node slot storage and relocation make `btree::map` safe under allocation failure for string keys, with no CPU or memory regression?

Result: yes on the measured configurations. Base is 25f9494 (`btree.h` sha256 `94c8a431…`), candidate is 7130d24 (`btree.h` `4bbc951c…`), benchmark sources at 01c6281 plus the files in this commit. Apple M4 Pro, Apple clang 17.0.0, `-std=c++20 -O3 -DNDEBUG`.

## Cause and change

Map slots held `std::pair<const Key, T>`; relocation moved the pair, which copies the const key. With heap-allocated keys every split, rebalance, merge, erase and root swap allocated, and a refusal mid-way left the tree half reshaped. The change stores a mutable `std::pair<Key, T>`, exposes it as `pair<const Key, T>&` through a union, relocates by real moves, allocates every node before the first value move, and builds the incoming value before reshaping. See `docs/NODE_LAYOUT.md`, "Value slots and relocation".

## Verification

| Check | Base 25f9494 | Candidate |
| --- | --- | --- |
| Detent `actual_sweep` (knobs/patch/group/cli × insert/erase/copy/destroy × short/long, max_n 40 and 100) | long insert 33-65 violations, long erase 3-4, rc 134 | 0 violations, rc 0, all 64 cells |
| Erase of 65,536 entries with 200-byte keys, heap calls | 72,692 | 0 |
| `tests/relocation_failure.cc` (refuse every allocation point of insert; erase with the heap closed) | fails (live allocations differ after refusal; erase allocates) | passes |
| `tests/run_layout.sh` (clang C++17/20, release/debug; four fixtures per profile) | not re-run | 16/16 |
| `tests/run_mixed_mode.sh` | not re-run | 16/16 |
| g++-16: `run_layout.sh` C++17 both modes, C++20 `node_layout`, `ownership_move`; relocation fixture C++17/20; g++-13 relocation fixture C++17/20 | not run | passes, except note below |
| UBSan+object-size+pointer-overflow (no ASan), all four fixtures, C++20, both modes | not run | passes |
| `CXX=clang++ BTREE_TEST_SANITIZE=1` variants | not run: ASan hangs on host (a trivial `-fsanitize=address` hello does not exit in 20 s) | not run: ASan hangs on host |

Revert check: the fixture built against 25f9494 (`-DRELOCATION_SKIP_MOVE_ONLY_KEYS`, because the base cannot compile move-only map keys) aborts with `live_allocations == before` after a refused insertion. With insertion refusals disabled it aborts on `allocation_calls == 0` in the first erase.

Existing fixtures changed where they asserted the old mechanism: `prepared_key_failures` now requires shifted keys to be moved (every copy and allocation refused, insertion succeeds), and the 243-byte mapped value case now inserts without allocating. The removed code is the per-key copy preparation for const-key pairs.

GCC 16.0 at `-std=c++20 -O2 -DNDEBUG` aborts in `tests/prepared_commit.cc` on both the base and the candidate; it passes at -O0/-O1 and with g++-13. It is not attributed to this change and was not investigated.

## Measurement

Five alternating pairs under `quiet exclusive btree-move 45`, `python3 .scratch/bench/ab.py 5` (alternation order flips each pair). A/A first (base against base, 200-byte keys): medians 256.4/249.4 ms; ratio 0.973, so the floor is about ±3%, individual runs differ by up to 10%.

`mutation_bench` (65,536 random inserts then erases, 80-byte mapped strings, 4 cycles), process CPU ms:

| Key bytes | Base median [min..max] | Candidate median [min..max] | new/base |
| --- | --- | --- | --- |
| 12 (SSO) | 88.2 [88.0..88.9] | 88.7 [86.6..102.1] | 1.006 |
| 16 (SSO on libc++) | 96.4 [95.8..100.4] | 96.6 [95.5..97.7] | 1.003 |
| 32 | 163.1 [162.6..183.0] | 120.7 [119.4..121.6] | 0.740 |
| 200 | 249.5 [242.0..266.5] | 175.4 [168.3..186.7] | 0.703 |
| `--copy` | 12.3 | 12.3 | 1.003 |
| `--copy-pruned` | 2.8 | 2.8 | 1.012 |

Keys of 16 bytes fit libc++'s 22-byte small buffer, so they equal the 12-byte row; they exceed libstdc++'s 15-byte buffer and should behave like the 32-byte row there (not measured). The 12-byte candidate maximum is a single outlier (paired ratios 0.983..1.160, median 1.006); the three SSO/copy rows are indistinguishable from noise.

`mutation_allocation_bench` phase CPU ms (medians of five, node allocations metered): random-insert short 11.4 to 11.3, random-erase short 10.2 to 10.2; long-random insert 27.4 to 19.6, erase 24.7 to 21.9; long-ordered insert 6.3 to 4.6, erase 8.0 to 5.5; generic-long-random insert 31.9 to 24.6, erase 23.4 to 20.4; nested-8 insert 8.5 to 5.7, erase 4.4 to 3.9.

Allocations (deterministic, `relocation_alloc_bench`, global `operator new` calls, 65,536 entries, 80-byte mapped strings):

| Key bytes | Insert calls base to candidate | Insert bytes | Erase calls |
| --- | --- | --- | --- |
| 12 or 16 | 82,130 to 82,123 | 10,173,672 to 10,173,056 | 0 to 0 |
| 200 | 420,230 to 147,659 | 80,498,472 to 23,804,544 | 72,692 to 0 |

`mutation_allocation_bench` node allocation calls, requested bytes and peak live bytes are identical in all workloads except nested-8 (51,271 to 51,236 calls; 10,521,120 to 10,514,960 bytes). Node layout size is unchanged (`sizeof` of the two pair types is asserted equal).

## Limits

- Release only; Debug (`-UNDEBUG`) timing was not measured.
- One machine, one compiler for timing. No Linux GCC/Clang sanitizer matrix, no FreeBSD, no downstream build beyond Detent's four model tables in `actual_sweep`.
- The two-pair union is implementation-defined; it is gated by static assertions on size, alignment and mirror-struct member offsets, with no fallback.
- Types whose move may throw keep the old guarantee: allocation ordering still helps, but a throwing relocation midway through a split, rebalance or merge is not recovered.
- Sets are unchanged.
