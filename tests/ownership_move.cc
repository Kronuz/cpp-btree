#include "btree/map.h"
#include "btree/set.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>
#include <type_traits>
#include <vector>

namespace {
void check(bool condition) { if (!condition) std::abort(); }
struct Refused {};
struct State {
    bool allocate = true, copy = true, assign = true, reverse = false;
    std::map<void*, std::size_t> blocks;
};
template <class T> struct Allocator {
    using value_type = T;
    std::shared_ptr<State> state;
    explicit Allocator(std::shared_ptr<State> owner) noexcept : state(std::move(owner)) {}
    Allocator(const Allocator& other) {
        if (!other.state->copy) throw Refused{};
        state = other.state;
    }
    template <class U> Allocator(const Allocator<U>& other) {
        if (!other.state->copy) throw Refused{};
        state = other.state;
    }
    Allocator& operator=(const Allocator& other) {
        if (!other.state->assign) throw Refused{};
        state = other.state; return *this;
    }
    T* allocate(std::size_t count) {
        if (!state->allocate) throw Refused{};
        auto* result = std::allocator<T>{}.allocate(count);
        try { check(state->blocks.emplace(result, count * sizeof(T)).second); }
        catch (...) { std::allocator<T>{}.deallocate(result, count); throw; }
        return result;
    }
    void deallocate(T* pointer, std::size_t count) noexcept {
        auto found = state->blocks.find(pointer);
        check(found != state->blocks.end() && found->second == count * sizeof(T));
        state->blocks.erase(found); std::allocator<T>{}.deallocate(pointer, count);
    }
    template <class U> bool operator==(const Allocator<U>& other) const noexcept { return state == other.state; }
    template <class U> bool operator!=(const Allocator<U>& other) const noexcept { return state != other.state; }
};
struct alignas(64) Value {
    inline static std::set<const Value*> live;
    inline static bool copy = true;
    int number;
    explicit Value(int v) : number(v) { check(live.insert(this).second); }
    Value(const Value& other) : number(other.number) {
        if (!copy) throw Refused{};
        check(live.insert(this).second);
    }
    Value(Value&& other) : Value(static_cast<const Value&>(other)) {}
    Value& operator=(const Value& other) { if (!copy) throw Refused{}; number = other.number; return *this; }
    Value& operator=(Value&& other) { return *this = static_cast<const Value&>(other); }
    ~Value() { check(live.erase(this) == 1); }
};
struct Compare {
    std::shared_ptr<State> state;
    explicit Compare(std::shared_ptr<State> s) noexcept : state(std::move(s)) {}
    Compare(const Compare& other) {
        if (!other.state->copy) throw Refused{};
        state = other.state;
    }
    Compare& operator=(const Compare& other) {
        if (!other.state->assign) throw Refused{};
        state = other.state; return *this;
    }
    static int key(int value) { return value; }
    static int key(const Value& value) { return value.number; }
    template <class T> bool operator()(const T& a, const T& b) const {
        return state->reverse ? key(a) > key(b) : key(a) < key(b);
    }
};
template <class Tree> void put(Tree& tree, int key) {
    if constexpr (std::is_same<typename Tree::key_type, typename Tree::value_type>::value)
        tree.insert(Value(key));
    else tree.insert(typename Tree::value_type(key, Value(key)));
}
template <class Tree> auto addresses(const Tree& tree) {
    std::vector<const typename Tree::value_type*> result;
    for (const auto& entry : tree) result.push_back(&entry);
    return result;
}
template <class Tree, bool Multi> void exercise() {
    static_assert(!std::is_nothrow_move_constructible<Tree>::value);
    static_assert(!std::is_nothrow_move_assignable<Tree>::value);
    for (bool reverse : {false, true}) for (int count : {0, 1, 3, 80}) {
        auto source_state = std::make_shared<State>();
        auto old_state = std::make_shared<State>();
        auto source_compare = std::make_shared<State>(); source_compare->reverse = reverse;
        auto old_compare = std::make_shared<State>(); old_compare->reverse = !reverse;
        {
            Tree source{Compare(source_compare), typename Tree::allocator_type(source_state)};
            for (int i = 0; i < count; ++i) put(source, i);
            if constexpr (Multi) if (count) put(source, count / 2);
            const auto original = addresses(source);
            const auto objects = Value::live;
            const auto blocks = source_state->blocks;
            source_state->allocate = false; Value::copy = false;
            Tree moved(std::move(source));
            check(source.empty() && addresses(moved) == original && Value::live == objects && source_state->blocks == blocks);
            source_state->copy = source_state->assign = false;
            source_compare->copy = source_compare->assign = false;
            moved = std::move(moved);
            source_state->copy = source_state->assign = true;
            source_compare->copy = source_compare->assign = true;
            check(addresses(moved) == original && Value::live == objects);
            source = std::move(moved);
            check(moved.empty() && addresses(source) == original && Value::live == objects);
            Value::copy = true; source_state->allocate = true;
            put(moved, 1000); moved.verify(); check(!moved.empty());
            Tree destination{Compare(old_compare), typename Tree::allocator_type(old_state)};
            put(destination, 500);
            source_state->allocate = false; Value::copy = false;
            destination = std::move(source);
            check(source.empty() && addresses(destination) == original && old_state->blocks.empty());
            Value::copy = true; source_state->allocate = true;
            if (count == 0) {
                const auto owned = source_state->blocks.size();
                put(destination, 1002);
                check(source_state->blocks.size() > owned && old_state->blocks.empty());
            }
            source.verify(); destination.verify();
            if (count > 0) {
                if constexpr (std::is_same<typename Tree::key_type, typename Tree::value_type>::value)
                    check(destination.find(Value(0)) != destination.end());
                else check(destination.find(0) != destination.end());
            }
            const Tree& immutable = destination;
            const auto unchanged = addresses(destination);
            Tree copied(std::move(immutable));
            check(addresses(destination) == unchanged && copied.size() == destination.size());
            check(addresses(copied) != unchanged);
            Tree assigned{Compare(old_compare), typename Tree::allocator_type(old_state)};
            put(assigned, 700); assigned = std::move(immutable);
            check(addresses(destination) == unchanged && assigned.size() == destination.size());
            check(addresses(assigned) != unchanged && old_state->blocks.empty());
            put(source, 1001); source.verify();
        }
        check(source_state->blocks.empty() && old_state->blocks.empty() && Value::live.empty());
        for (int failure = 0; failure < 4; ++failure) {
            Tree source{Compare(source_compare), typename Tree::allocator_type(source_state)};
            Tree destination{Compare(old_compare), typename Tree::allocator_type(old_state)};
            for (int i = 0; i < count; ++i) put(source, i);
            put(destination, 500);
            const auto original = addresses(source);
            const auto blocks = source_state->blocks;
            if (failure == 0) source_compare->copy = false;
            if (failure == 1) source_state->copy = false;
            if (failure == 2) source_compare->assign = false;
            if (failure == 3) source_state->assign = false;
            bool refused = false;
            try {
                if (failure < 2) { Tree rejected(std::move(source)); }
                else destination = std::move(source);
            } catch (const Refused&) { refused = true; }
            source_compare->copy = source_compare->assign = true;
            source_state->copy = source_state->assign = true;
            check(refused && addresses(source) == original && source_state->blocks == blocks);
            if (failure >= 2) check(destination.empty() && old_state->blocks.empty());
            source.verify(); destination.verify();
            put(destination, 1002); destination.verify();
        }
        check(source_state->blocks.empty() && old_state->blocks.empty() && Value::live.empty());
    }
}
struct DerivedCompare {
    inline static bool reject = false;
    inline static int selected = 0;
    DerivedCompare() = default;
    DerivedCompare(const DerivedCompare&) noexcept = default;
    template <class T, std::enable_if_t<std::is_base_of<DerivedCompare, T>::value && !std::is_same<DerivedCompare, T>::value, int> = 0>
    DerivedCompare(const T&) { ++selected; if (reject) throw Refused{}; }
    bool operator()(int a, int b) const { return a < b; }
};
void derived_selection() {
    using Tree = btree::map<int, int, DerivedCompare>;
    static_assert(std::is_nothrow_copy_constructible<DerivedCompare>::value);
    static_assert(!std::is_nothrow_move_constructible<Tree>::value);
    Tree source; source.emplace(1, 1); source.emplace(2, 2);
    const auto before = addresses(source);
    const auto calls = DerivedCompare::selected;
    DerivedCompare::reject = true; bool refused = false;
    try { Tree rejected(std::move(source)); } catch (const Refused&) { refused = true; }
    DerivedCompare::reject = false;
    check(refused && DerivedCompare::selected > calls && addresses(source) == before);
    Tree moved(std::move(source)); check(source.empty() && addresses(moved) == before); moved.verify();
}
struct AddressCompare {
    inline static int calls = 0;
    inline static bool reject = false;
    const void* operator&() const {
        ++calls;
        if (reject) throw Refused{};
        return nullptr;
    }
    bool operator()(int a, int b) const { return a < b; }
};
template <class Tree> void address_selection() {
    static_assert(std::is_nothrow_move_constructible<Tree>::value);
    static_assert(std::is_nothrow_move_assignable<Tree>::value);
    Tree source;
    if constexpr (std::is_same<typename Tree::key_type, typename Tree::value_type>::value)
        source.insert(1);
    else source.insert(typename Tree::value_type(1, 2));
    const auto before = addresses(source);
    AddressCompare::calls = 0;
    AddressCompare::reject = true;
    source = std::move(source);
    check(addresses(source) == before);
    source = static_cast<const Tree&>(source);
    check(addresses(source) == before);
    Tree target;
    target = std::move(source);
    check(source.empty() && addresses(target) == before);
    source = target;
    check(source.size() == target.size() && AddressCompare::calls == 0);
    AddressCompare::reject = false;
}
}
int main() {
    try {
        using Pair = std::pair<const int, Value>;
        exercise<btree::map<int, Value, Compare, Allocator<Pair>>, false>();
        exercise<btree::multimap<int, Value, Compare, Allocator<Pair>>, true>();
        exercise<btree::set<Value, Compare, Allocator<Value>>, false>();
        exercise<btree::multiset<Value, Compare, Allocator<Value>>, true>();
        derived_selection();
        static_assert(std::is_nothrow_move_constructible<btree::map<int, Value>>::value);
        static_assert(std::is_nothrow_move_assignable<btree::map<int, Value>>::value);
        static_assert(std::is_nothrow_move_constructible<btree::set<int>>::value);
        static_assert(std::is_nothrow_move_assignable<btree::set<int>>::value);
        address_selection<btree::map<int, int, AddressCompare>>();
        address_selection<btree::multimap<int, int, AddressCompare>>();
        address_selection<btree::set<int, AddressCompare>>();
        address_selection<btree::multiset<int, AddressCompare>>();
    } catch (...) { std::puts("FAIL ownership move exception"); return 1; }
    std::puts("PASS ownership moves, exact identities, allocator and comparator failures");
}
