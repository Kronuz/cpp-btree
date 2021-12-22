#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <rapidcheck.h>
#include <utility>

#include <algorithm>
#include <btree/btree.hpp>
#include <vector>

// Snippet from:
// https://gist.github.com/objectx/8279b3bf1cba6864a1b4fee24fb5dfc1
namespace rc {
template <typename Testable_>
void prop(const std::string& description, Testable_&& testable) {
    using namespace detail;
    DOCTEST_SUBCASE(description.c_str()) {
        const auto result = checkTestable(std::forward<Testable_>(testable));

        if (result.template is<SuccessResult>()) {
            const auto success = result.template get<SuccessResult>();
            if (!success.distribution.empty()) {
                std::cout << "- " << description << '\n';
                printResultMessage(result, std::cout);
                std::cout << std::endl;
            }
        } else {
            std::ostringstream ss;
            printResultMessage(result, ss);
            DOCTEST_FAIL_CHECK(ss.str() << "\n");
        }
    }
}
} // namespace rc

TEST_CASE("double reversal yields the original value") {
    rc::check("double reversal yields the original value",
              [](const std::vector<int>& l0) {
                  auto l1 = l0;
                  std::reverse(begin(l1), end(l1));
                  std::reverse(begin(l1), end(l1));
                  RC_ASSERT(l0 == l1);
              });
}