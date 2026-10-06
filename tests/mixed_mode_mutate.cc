#include "mixed_mode_types.hh"

void fill(Map &map, unsigned start, unsigned count) {
	for (unsigned i = start; i < start + count; ++i)
		map.emplace(key(i), Value{std::in_place_type<Tracked>, key(i) + std::string(80, 'x')});
}
void prune(Map &map, unsigned count) {
	while (count-- && !map.empty())
		map.erase(map.begin());
}
void clear_elsewhere(Map &map) { map.clear(); }
