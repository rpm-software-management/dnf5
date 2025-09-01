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

#ifndef LIBDNF5_TEST_ARCH_HPP
#define LIBDNF5_TEST_ARCH_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class ArchTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(ArchTest);
    CPPUNIT_TEST(test_get_base_arch);
    CPPUNIT_TEST(test_get_supported_arches);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_get_base_arch();
    void test_get_supported_arches();
};

#endif  // TEST_LIBDNF5_RPM_ARCH_HPP
