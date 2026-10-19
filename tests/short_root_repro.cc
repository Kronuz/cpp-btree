#include "btree/map.h"
#include <string>

int main() {
	btree::map<std::string, int> values;
	values.emplace("first", 1);
	if (values.at("first") != 1) return 1;
	values.emplace("second", 2);
	values.erase("first");
	values.verify();
	return values.size() == 1 && values.at("second") == 2 ? 0 : 1;
}
