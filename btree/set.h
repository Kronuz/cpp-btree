/*
 * LICENSE
 *
 * Copyright (c) 2021 Anguilla Team
 * Copyright (c) 2019 German Mendez Bravo (Kronuz)
 * Copyright (c) 2013 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * DESCRIPTION
 *
 * A btree::set<> implements the STL unique sorted associative container
 * interface (a.k.a set<>) using a B-tree.
 * See btree.h for details of the B-tree implementation and caveats.
 */

#pragma once

#ifndef BTREE_SET_H__
#define BTREE_SET_H__

#include "btree.h"

namespace btree {

// The set class is needed mainly for its constructors.
template <typename Key, typename Compare = std::less<Key>,
          typename Alloc = std::allocator<Key>, int TargetNodeSize = 256>
class set : public btree_unique_container<
                btree<btree_set_params<Key, Compare, Alloc, TargetNodeSize>>> {
    typedef set<Key, Compare, Alloc, TargetNodeSize> self_type;
    typedef value_type Key;
    typedef btree_set_params<Key, Compare, Alloc, TargetNodeSize> params_type;
    typedef btree<params_type> btree_type;
    typedef btree_unique_container<btree_type> super_type;

  public:
    typedef typename btree_type::key_compare key_compare;
    typedef typename btree_type::allocator_type allocator_type;
    static_assert(
        (std::is_same<typename allocator_type::value_type, value_type>::value),
        "Alloc::value_type must be same type as value_type");

  public:
    // Default constructors.
    set() : super_type(key_compare(), allocator_type()) {}

    explicit set(const key_compare& comp)
        : super_type(comp, allocator_type()) {}

    explicit set(const allocator_type& alloc)
        : super_type(key_compare(), alloc) {}

    explicit set(const key_compare& comp, const allocator_type& alloc)
        : super_type(comp, alloc) {}

    // Copy constructor.
    set(const self_type& x) : super_type(x) {}

    // Range constructor.
    template <class InputIterator>
    set(InputIterator b, InputIterator e,
        const key_compare& comp = key_compare(),
        const allocator_type& alloc = allocator_type())
        : super_type(b, e, comp, alloc) {}
};

} // namespace btree

template <typename K, typename C, typename A, int N>
bool operator==(const btree::set<K, C, A, N>& lhs,
                const btree::set<K, C, A, N>& rhs) {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename K, typename C, typename A, int N>
bool operator<(const btree::set<K, C, A, N>& lhs,
               const btree::set<K, C, A, N>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename K, typename C, typename A, int N>
bool operator!=(const btree::set<K, C, A, N>& lhs,
                const btree::set<K, C, A, N>& rhs) {
    return !(lhs == rhs);
}

template <typename K, typename C, typename A, int N>
bool operator>(const btree::set<K, C, A, N>& lhs,
               const btree::set<K, C, A, N>& rhs) {
    return rhs < lhs;
}

template <typename K, typename C, typename A, int N>
bool operator>=(const btree::set<K, C, A, N>& lhs,
                const btree::set<K, C, A, N>& rhs) {
    return !(lhs < rhs);
}

template <typename K, typename C, typename A, int N>
bool operator<=(const btree::set<K, C, A, N>& lhs,
                const btree::set<K, C, A, N>& rhs) {
    return !(rhs < lhs);
}

template <typename K, typename C, typename A, int N>
inline void swap(btree::set<K, C, A, N>& x, btree::set<K, C, A, N>& y) {
    x.swap(y);
}

namespace btree {

// The multiset class is needed mainly for its constructors.
template <typename Key, typename Compare = std::less<Key>,
          typename Alloc = std::allocator<Key>, int TargetNodeSize = 256>
class multiset
    : public btree_multi_container<
          btree<btree_set_params<Key, Compare, Alloc, TargetNodeSize>>> {
    typedef multiset<Key, Compare, Alloc, TargetNodeSize> self_type;
    typedef btree_set_params<Key, Compare, Alloc, TargetNodeSize> params_type;
    typedef btree<params_type> btree_type;
    typedef btree_multi_container<btree_type> super_type;

  public:
    typedef typename btree_type::key_compare key_compare;
    typedef typename btree_type::allocator_type allocator_type;

  public:
    // Default constructors.
    multiset() : super_type(key_compare(), allocator_type()) {}

    explicit multiset(const key_compare& comp)
        : super_type(comp, allocator_type()) {}

    explicit multiset(const allocator_type& alloc)
        : super_type(key_compare(), alloc) {}

    explicit multiset(const key_compare& comp, const allocator_type& alloc)
        : super_type(comp, alloc) {}

    // Copy constructor.
    multiset(const self_type& x) : super_type(x) {}

    // Copy constructor.
    multiset(const self_type& x) : super_type(x) {}

    // Range constructor.
    template <class InputIterator>
    multiset(InputIterator b, InputIterator e,
             const key_compare& comp = key_compare(),
             const allocator_type& alloc = allocator_type())
        : super_type(b, e, comp, alloc) {}
};

} // namespace btree

template <typename K, typename C, typename A, int N>
bool operator==(const btree::multiset<K, C, A, N>& lhs,
                const btree::multiset<K, C, A, N>& rhs) {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename K, typename C, typename A, int N>
bool operator<(const btree::multiset<K, C, A, N>& lhs,
               const btree::multiset<K, C, A, N>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename K, typename C, typename A, int N>
bool operator!=(const btree::multiset<K, C, A, N>& lhs,
                const btree::multiset<K, C, A, N>& rhs) {
    return !(lhs == rhs);
}

template <typename K, typename C, typename A, int N>
bool operator>(const btree::multiset<K, C, A, N>& lhs,
               const btree::multiset<K, C, A, N>& rhs) {
    return rhs < lhs;
}

template <typename K, typename C, typename A, int N>
bool operator>=(const btree::multiset<K, C, A, N>& lhs,
                const btree::multiset<K, C, A, N>& rhs) {
    return !(lhs < rhs);
}

template <typename K, typename C, typename A, int N>
bool operator<=(const btree::multiset<K, C, A, N>& lhs,
                const btree::multiset<K, C, A, N>& rhs) {
    return !(rhs < lhs);
}

template <typename K, typename C, typename A, int N>
inline void swap(btree::multiset<K, C, A, N>& x,
                 btree::multiset<K, C, A, N>& y) {
    x.swap(y);
}

#endif // BTREE_SET_H__
