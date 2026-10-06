#pragma once

#include <btree/map.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

inline void require(bool condition) {
	if (!condition)
		throw std::runtime_error("mixed-build B-tree regression");
}
struct Tracked {
	inline static int live = 0;
	std::string text;
	explicit Tracked(std::string value) : text(std::move(value)) { ++live; }
	Tracked(const Tracked &other) : text(other.text) { ++live; }
	Tracked(Tracked &&other) noexcept : text(std::move(other.text)) { ++live; }
	Tracked &operator=(const Tracked &) = default;
	Tracked &operator=(Tracked &&) noexcept = default;
	~Tracked() { --live; }
};
using Value = std::variant<bool, unsigned char, Tracked>;
using Map = btree::map<std::string, Value, std::less<>>;
inline std::string key(unsigned number) { return "group" + std::to_string(number); }
void fill(Map &map, unsigned start, unsigned count);
void prune(Map &map, unsigned count);
void clear_elsewhere(Map &map);
