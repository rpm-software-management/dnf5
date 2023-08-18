//
// Created by charlie on 8/18/23.
//

#ifndef LIBDNF5_TEST_REPO_SACK_H
#define LIBDNF5_TEST_REPO_SACK_H

#include "../../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>

class RepoSackTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RepoSackTest);
    CPPUNIT_TEST(test_call_twice_fails);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_call_twice_fails();
};


#endif  // LIBDNF5_TEST_REPO_SACK_H
