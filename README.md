# btree

Part of [Kronuz C++ Libraries](https://github.com/Kronuz/cpp-libraries), a collection of small, reusable C++ libraries.

[![CI](https://github.com/Kronuz/btree/actions/workflows/ci.yml/badge.svg)](https://github.com/Kronuz/btree/actions/workflows/ci.yml)

![C++17](https://img.shields.io/badge/C%2B%2B-17%20%7C%2020-blue.svg)
![Header-only](https://img.shields.io/badge/header--only-yes-green.svg)
[![License: Apache 2.0](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)

`std::map` and `std::set`, but B-trees: in-memory ordered containers that use a
fraction of the memory and fewer cache misses. Based on
[Google's C++ B-tree](https://code.google.com/archive/p/cpp-btree/), modernized to
behave like the C++17 standard containers.

## ✨ Highlights

- 🧠 **Less memory**: about 1 byte of overhead per `int32_t` in `btree::set`, against
  16 bytes for a red-black tree node on a 32-bit system.
- ⚡ **Cache-friendly**: several key comparisons per node, so large containers
  search faster.
- 🔁 **Near drop-in**: `btree::map`, `set`, `multimap`, and `multiset` with
  `emplace`, `try_emplace`, heterogeneous lookup, and values that need no default
  constructor.
- ⚠️ **One difference**: any insert or erase invalidates every iterator into the
  container.

## 📦 Installation

Header-only and dependency-free: copy the `btree/` directory onto your include path.
It builds as C++17 or C++20. CMake consumers can use `add_subdirectory` or FetchContent and link `btree::btree`; tests build only when this repository is the top-level project.

## 🚀 Quick start

```cpp
#include "btree/map.h"

#include <cassert>
#include <string>

int main() {
	btree::map<int, std::string> m;
	m.emplace(2, "two");
	m.try_emplace(1, "one");
	assert(m.begin()->second == "one");   // ordered, like std::map
	assert(m.count(2) == 1);
}
```

## 🎯 When to use it

- ✅ Large ordered maps and sets where memory or lookup speed matters.
- ❌ Code that keeps iterators or references across inserts and erases: use
  `std::map`.

## 📚 Documentation

- [docs/TESTING.md](docs/TESTING.md): the mixed-mode regression script, sanitizers,
  and the recorded benchmark comparison.
- [docs/NODE_LAYOUT.md](docs/NODE_LAYOUT.md): compact node storage, slot lifetimes,
  alignment and allocator compatibility.

## 🛠️ Testing

```sh
sh tests/run_mixed_mode.sh                         # release/debug TU mixes, C++17 and C++20
CXX=clang++ BTREE_TEST_SANITIZE=1 sh tests/run_mixed_mode.sh   # with ASan and UBSan
sh tests/run_layout.sh                             # compact roots and counting allocators
CXX=clang++ BTREE_TEST_SANITIZE=1 sh tests/run_layout.sh
```

## 🤝 Contributing

Issues and pull requests are welcome.

## 📄 License

Apache 2.0. See [LICENSE](LICENSE) and [NOTICE](NOTICE). Copyright (c) 2013 Google Inc. and (c) 2019-2026 Germán Méndez Bravo.

Range insertion and range construction refresh their end hint after each insertion, including root growth and splits. The compact-layout suite compares all four container families with standard ordered containers for duplicate input and populated destinations.

Comparator return-type checking uses const references and does not require a default-constructible key. Comparators still return `bool`, or `int` when using the compare-to tag.

First insertion publishes its root only after constructing the value. Allocation or value-construction failure leaves a reusable empty container. This narrow guarantee does not extend to all later split or rebalance failures.

Copies construct a compact ordered shape without key comparisons and release every completed value and node on failure, including nested containers. Copy assignment retains clear-then-copy behavior: a failed copy leaves the destination empty, and does not preserve its prior contents. Other mutation failure guarantees are unchanged.

Known limitation: nested containers with non-default-constructible allocators can fail constructor-trait instantiation under Clang with libstdc++ 13 in C++20. The ordered-copy repair does not change constructor constraints; this remains a separate compatibility issue.

Compact leaf-root growth constructs the incoming value first in its final replacement slot, then relocates existing values under counted ownership. Failed copying leaves original contents intact; throwing move-only mapped values can remain moved-from. After a destructive move of a noncopyable set key fails, the set becomes empty and reusable. Explicit moves from contained mapped arguments may consume them. Other insertion and erase paths retain their existing limitations.

## Repository compatibility

This repository was renamed from `Kronuz/cpp-btree` to `Kronuz/btree`. GitHub redirects old repository and clone URLs; the history and stars stay with this repository. Keep the old repository name unused to preserve those redirects. Header paths remain `btree/map.h` and `btree/set.h`. The original `cpp_btree::cpp_btree` CMake target remains an alias of the same interface target as `btree::btree`.

Nonfull leaf insertion publishes the internal-root size after successful value construction. Refused append leaves the previous size intact and retry reuses existing capacity. Full-node split failure and general mutation safety remain separate qualification work.

Nonfull middle insertion reconstructs actual const-key pairs after preparing fallible work. It uses bounded automatic storage for eligible nonthrowing transfers and the existing guarded replacement mechanism for generic cases. Tail append remains direct. Full-node mutation and erasure safety remain separate work; this correction does not promise failure atomicity for those paths.

The [matched Detent report](https://github.com/Kronuz/Detent/blob/main/benchmarks/btree-prepared-insertion-20261022.json) records retained/peak bytes and CPU, including explicit long-key and generic-fallback insertion costs. `benchmarks/mutation_bench.cc` supports `--copy`, `--copy-pruned` and `--move-nested`; set `-DBTREE_BENCH_CYCLES=64` for longer runs. `mutation_allocation_bench.cc` meters node requests, excluding string heaps; Detent's actual-model probe includes string allocations. Compare identical compiler flags and sources under an isolated measurement window. The optional nested allocator benchmark remains subject to the separately documented legacy constructor-trait limitation on some compiler/library combinations.
