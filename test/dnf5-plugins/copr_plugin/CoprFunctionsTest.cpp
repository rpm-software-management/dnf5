// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "CoprFunctionsTest.hpp"

#include "helpers.hpp"

using namespace std;

void CoprFunctionsTest::test_repo_fallbacks() {
    CPPUNIT_ASSERT(vector<string>{"epel-8"} == dnf5::repo_fallbacks("epel-8"));
    CPPUNIT_ASSERT((vector<string>{"almalinux-10", "epel-10"} == dnf5::repo_fallbacks("almalinux-10")));
    CPPUNIT_ASSERT((vector<string>{"rhel-10.4", "rhel-10", "epel-10"} == dnf5::repo_fallbacks("rhel-10.4")));
}

CPPUNIT_TEST_SUITE_REGISTRATION(CoprFunctionsTest);
