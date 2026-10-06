#include "mixed_mode_types.hh"
#include <map>
#include <random>

int main() {
	{
		Map map;
		fill(map, 0, 4096);
		map.verify();
		require(map.size() == 4096 && Tracked::live == 4096);
		while (!map.empty()) {
			map.erase(map.begin());
			map.verify();
			require(Tracked::live == static_cast<int>(map.size()));
		}
		for (unsigned iteration = 0; iteration < 8; ++iteration) {
			fill(map, 0, 512);
			prune(map, 256);
			map.verify();
			require(map.size() == 256 && Tracked::live == 256);
			clear_elsewhere(map);
			require(Tracked::live == 0);
			for (unsigned i = 0; i < 512; ++i)
				map.emplace(key(i), Value{std::in_place_type<Tracked>, key(i) + std::string(80, 'x')});
			prune(map, 512);
			require(map.empty() && Tracked::live == 0);
		}
		std::map<std::string, std::string> oracle;
		std::mt19937 random(7106);
		for (unsigned i = 0; i < 20000; ++i) {
			auto number = random() % 2048;
			if (random() & 1) {
				fill(map, number, 1);
				oracle.emplace(key(number), key(number) + std::string(80, 'x'));
			} else {
				map.erase(key(number));
				oracle.erase(key(number));
			}
			map.verify();
			require(map.size() == oracle.size() && Tracked::live == static_cast<int>(map.size()));
			if (!(i % 128)) {
				auto expected = oracle.begin();
				for (const auto &entry : map) {
					require(expected != oracle.end() && entry.first == expected->first &&
							std::get<Tracked>(entry.second).text == expected->second);
					++expected;
				}
				require(expected == oracle.end());
			}
		}
	}
	require(Tracked::live == 0);
}
