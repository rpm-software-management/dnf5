// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_REPO_QUERY_HPP
#define LIBDNF5_TEST_REPO_QUERY_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class RepoQueryTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RepoQueryTest);
    CPPUNIT_TEST(test_query_basics);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_query_basics();
};

#endif
