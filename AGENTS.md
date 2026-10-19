# Working notes

This is a dependency-free, header-only C++17/C++20 ordered-container library. The implementation lives in `btree/btree.h`; `btree/map.h` and `btree/set.h` expose the containers. Read `docs/NODE_LAYOUT.md` before changing node storage.

Keep compact root allocations. Every allocation contains a complete fixed header and separately constructed trailing slots; full leaf/internal/root descriptor objects are never placed in short storage. Derive allocation, access, deallocation and byte statistics from the same layout helpers. Preserve value and pointer alignment, live value counts and matching allocator type/count pairs.

Inactive child values remain unspecified in both Debug and Release. Changing diagnostics must not make mutation behavior depend on `NDEBUG`. Inserts and erases invalidate iterators as documented; do not promise STL node-address stability.

Run `sh tests/run_layout.sh` and `sh tests/run_mixed_mode.sh`, then both with `CXX=clang++ BTREE_TEST_SANITIZE=1`. Tests must exercise C++17/C++20 and Release/Debug with runtime checks active. Run downstream integration and before/after `benchmarks/mutation_bench.cc` under the workspace quiet protocol. The older mixed-mode baseline does not qualify later layout changes.

Use `.scratch/` for artifacts. Preserve Apache 2.0 source notices and public container APIs. Header revisions require a complete downstream rebuild; the mixed-mode guarantee applies to one revision built with different diagnostic flags.
