// Compile both revisions against this source with the same flags.
#include <btree/map.h>
#include <btree/set.h>
#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>

struct Counts {
	std::size_t live = 0, peak = 0, allocations = 0, deallocations = 0;
};
template <class T> struct Allocator {
	using value_type = T;
	std::shared_ptr<Counts> counts;
	explicit Allocator(std::shared_ptr<Counts> value) : counts(std::move(value)) {}
	template <class U> Allocator(const Allocator<U>& other) : counts(other.counts) {}
	T* allocate(std::size_t count) {
		auto pointer = std::allocator<T>{}.allocate(count);
		counts->live += count * sizeof(T);
		counts->peak = std::max(counts->peak, counts->live);
		++counts->allocations;
		return pointer;
	}
	void deallocate(T* pointer, std::size_t count) {
		counts->live -= count * sizeof(T);
		++counts->deallocations;
		std::allocator<T>{}.deallocate(pointer, count);
	}
	template <class U> bool operator==(const Allocator<U>& other) const { return counts == other.counts; }
	template <class U> bool operator!=(const Allocator<U>& other) const { return !(*this == other); }
};
template <class Tree> void report(const char* name, const Tree& tree, const Counts& counts) {
	std::printf("{\"case\":\"%s\",\"entries\":%zu,\"node_bytes\":%zu,\"peak_node_bytes\":%zu,"
		"\"allocations\":%zu,\"deallocations\":%zu,\"reported_node_bytes\":%zu}\n",
		name, static_cast<std::size_t>(tree.size()), counts.live, counts.peak,
		counts.allocations, counts.deallocations, static_cast<std::size_t>(tree.bytes_used() - sizeof(Tree)));
}
int main() {
	{
		auto counts = std::make_shared<Counts>();
		btree::set<int, std::less<int>, Allocator<int>> tree(std::less<int>{}, Allocator<int>{counts});
		report("int_set", tree, *counts);
		for (int i = 0; i < 4096; ++i) {
			tree.emplace(i);
			if (i < 4 || i == 4095) report("int_set", tree, *counts);
		}
		tree.clear();
		report("int_set", tree, *counts);
		if (counts->live || counts->allocations != counts->deallocations) return 1;
	}
	{
		auto counts = std::make_shared<Counts>();
		using Value = std::pair<const std::string, std::string>;
		btree::map<std::string, std::string, std::less<std::string>, Allocator<Value>> tree(
			std::less<std::string>{}, Allocator<Value>{counts});
		report("string_map", tree, *counts);
		for (int i = 0; i < 4096; ++i) {
			tree.emplace("key" + std::to_string(10000 + i), std::string(80, 'x'));
			if (i < 4 || i == 4095) report("string_map", tree, *counts);
		}
		tree.clear();
		report("string_map", tree, *counts);
		if (counts->live || counts->allocations != counts->deallocations) return 1;
	}
}
