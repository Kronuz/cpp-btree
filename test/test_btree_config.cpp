#include <doctest.h>

#include "btree/btree.hpp"
#include "btree/btree_config.hpp"

TEST_SUITE_BEGIN("core");

TEST_CASE("check version") {

#ifndef BTREE_VERSION_MAJOR
#error "BTREE_VERSION_MAJOR is undefined"
#endif

#ifndef BTREE_VERSION_MINOR
#error "BTREE_VERSION_MINOR is undefined"
#endif

#ifndef BTREE_VERSION_PATCH
#error "BTREE_VERSION_PATCH is undefined"
#endif

    CHECK_EQ(BTREE_VERSION_MAJOR, 0);
    CHECK_EQ(BTREE_VERSION_MINOR, 1);
    CHECK_EQ(BTREE_VERSION_PATCH, 0);
}

TEST_SUITE_END(); // end of testsuite core
