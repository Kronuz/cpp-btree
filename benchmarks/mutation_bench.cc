// Compile with -O3 and either -DNDEBUG or -UNDEBUG. Repeat paired builds
// against the two include trees without concurrent builds or tests.
#include <algorithm>
#include <btree/map.h>
#include <cstdio>
#include <ctime>
#include <numeric>
#include <random>
#include <string>
#include <vector>

int main() {
	constexpr unsigned entries = 65536, cycles = 4;
	std::vector<std::string> keys;
	std::vector<unsigned> order(entries);
	std::iota(order.begin(), order.end(), 0);
	std::mt19937 random(7106);
	std::shuffle(order.begin(), order.end(), random);
	for (unsigned i = 0; i < entries; ++i)
		keys.push_back("group" + std::to_string(1000000 + i));
	const auto start = std::clock();
	for (unsigned cycle = 0; cycle < cycles; ++cycle) {
		btree::map<std::string, std::string> map;
		for (auto index : order)
			map.emplace(keys[index], keys[index] + std::string(80, 'x'));
		for (auto index : order)
			if (map.erase(keys[index]) != 1)
				return 1;
		if (!map.empty())
			return 1;
	}
	std::printf("{\"entries\":%u,\"cycles\":%u,\"process_cpu_ms\":%.6f}\n", entries, cycles,
				1000.0 * (std::clock() - start) / CLOCKS_PER_SEC);
}
