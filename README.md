# C++ B-tree

Code in this repository is based on
[Google's B-tree implementation](https://code.google.com/archive/p/cpp-btree/).

C++ B-tree is a template library that implements ordered in-memory containers
based on a B-tree data structure. Similar to the STL `std::map`, `std::set`,
`std::multimap`, and `std::multiset` templates, this library provides
`btree::map`, `btree::set`, `btree::multimap` and `btree::multiset`.

This difers from the original project by Google in that containers behave more
like modern STL (C++17) and are an almost drop-in replacements (except for the
iterator invalidation, see below); including support for `emplace` and
`try_emplace` as well as values in the map not needing to have a default
constructor.

C++ B-tree containers have a few advantages compared with the standard
containers, which are typically implemented using Red-Black trees. Nodes in a
Red-Black tree require three pointers per entry (plus 1 bit), whereas B-trees
on average make use of fewer than one pointer per entry, leading to
**significant memory savings**. For example, a `set<int32_t>` has an overhead
of 16 bytes for every 4 byte set element (on a 32-bit operating system); the
corresponding `btree::set<int32_t>` has an overhead of around 1 byte per set
element.

B-trees are widely known as data structures for secondary storage, because they
keep disk seeks to a minimum. For an in-memory data structure, the same property
yields a performance boost by keeping cache-line misses to a minimum. C++ B-tree
containers make better use of the cache by performing multiple key-comparisons
per node when searching the tree. Although B-tree algorithms are more complex,
compared with the Red-Black tree algorithms, the improvement in cache behavior
may account for a **significant speedup** in accessing large containers.

The C++ B-tree containers are not without drawbacks, however. Unlike the
standard STL containers, modifying a C++ B-tree container
**invalidates all outstanding iterators** on that container.

Run `sh tests/run_mixed_mode.sh` to check release/debug translation-unit combinations in both link orders under C++17 and C++20. Set `CXX=clang++ BTREE_TEST_SANITIZE=1` for ASan/UBSan. The regression exercises insertion, split, erase, merge, clear, verification and destruction with tracked nontrivial values. Inactive value storage and child slots are unspecified; diagnostics check active entries and child links without requiring debug-only zeroing. This removes coupling through diagnostic slot state, rather than promising compatibility for arbitrary compiler, allocator or standard-library configurations.

The [recorded comparison](benchmarks/mixed-mode-baseline.json) contains 16 native and 16 ASan/UBSan Linux regression cases and three local before/after benchmark pairs. Compile `benchmarks/mutation_bench.cc` with `-std=c++20 -O3`, the chosen include tree, and either `-DNDEBUG` or `-UNDEBUG`. The Release benchmark's disassembled text is identical before and after; timing variation is not attributed to this correction. No performance improvement is claimed.
