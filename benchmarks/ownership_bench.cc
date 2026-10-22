#include "allocation_meter.hh"
#include <cstdio>
#include <ctime>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <vector>

using Groups = btree_bench::Map<std::string>;
using Tree = btree_bench::Map<Groups>;
using Slot = std::aligned_storage_t<sizeof(Tree), alignof(Tree)>;

int main() {
	constexpr unsigned count = 128, entries = 512, groups = 3;
	for (unsigned phase = 0; phase != 3; ++phase) {
		auto meter = std::make_shared<btree_bench::AllocationMeter>();
		std::vector<Tree> sources;
		std::unique_ptr<Slot[]> slots(new Slot[count]);
		sources.reserve(count);
		for (unsigned index = 0; index != count; ++index) {
			sources.emplace_back(std::less<std::string>{}, Tree::allocator_type(meter));
			for (unsigned key = 0; key != entries; ++key) {
				Groups values(std::less<std::string>{}, Groups::allocator_type(meter));
				for (unsigned group = 0; group != groups; ++group)
					values.emplace(std::string(48, 'g') + std::to_string(group), std::string(80, 'v'));
				sources.back().emplace(std::string(48, 'k') + std::to_string(key), std::move(values));
			}
			if (phase != 0) {
				auto target = ::new (static_cast<void*>(&slots[index])) Tree(std::less<std::string>{}, Tree::allocator_type(meter));
				if (phase == 2) *target = sources.back();
			}
		}
		const auto calls = meter->calls, bytes = meter->requested, live = meter->live;
		meter->peak = live;
		const auto start = std::clock();
		for (unsigned index = 0; index != count; ++index) {
			if (phase == 0)
				::new (static_cast<void*>(&slots[index])) Tree(std::move(sources[index]));
			else
				*std::launder(reinterpret_cast<Tree*>(&slots[index])) = std::move(sources[index]);
		}
		const double elapsed = 1000.0 * (std::clock() - start) / CLOCKS_PER_SEC;
		for (unsigned index = 0; index != count; ++index) {
			auto target = std::launder(reinterpret_cast<Tree*>(&slots[index]));
			if (target->size() != entries) return 1;
			for (const auto& entry : *target) if (entry.second.size() != groups) return 1;
		}
		std::printf("{\"workload\":\"%s\",\"containers\":%u,\"entries_each\":%u,\"groups\":%u,\"process_cpu_ms\":%.6f,\"phase_allocation_calls\":%zu,\"phase_requested_bytes\":%zu,\"start_live_node_bytes\":%zu,\"end_live_node_bytes\":%zu,\"peak_live_node_bytes\":%zu}\n",
			phase == 0 ? "move-construct" : phase == 1 ? "move-assign-empty" : "move-assign-populated",
			count, entries, groups, elapsed, meter->calls - calls, meter->requested - bytes, live, meter->live, meter->peak);
		for (unsigned index = 0; index != count; ++index)
			std::launder(reinterpret_cast<Tree*>(&slots[index]))->~Tree();
		sources.clear();
		if (meter->live != 0) return 1;
	}
}
