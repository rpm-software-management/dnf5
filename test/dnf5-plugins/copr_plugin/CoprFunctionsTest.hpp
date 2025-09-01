// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5_TEST_COPR_FUNCTIONS_HPP
#define DNF5_TEST_COPR_FUNCTIONS_HPP

// Note 1
#include <cppunit/extensions/HelperMacros.h>

class CoprFunctionsTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(CoprFunctionsTest);
    CPPUNIT_TEST(test_repo_fallbacks);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() { unsetenv("TEST_COPR_CONFIG_DIR"); }

    void test_repo_fallbacks();
};

#endif  // DNF5_TEST_COPR_FUNCTIONS_HPP
