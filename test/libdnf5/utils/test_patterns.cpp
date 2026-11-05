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

#include "test_patterns.hpp"

#include <libdnf5/utils/patterns.hpp>

using namespace libdnf5::utils;

CPPUNIT_TEST_SUITE_REGISTRATION(UtilsPatternsTest);


void UtilsPatternsTest::test_is_file_pattern() {
    CPPUNIT_ASSERT(!is_file_pattern(""));
    CPPUNIT_ASSERT(!is_file_pattern("no_file_pattern"));
    CPPUNIT_ASSERT(!is_file_pattern("no_file/pattern"));
    CPPUNIT_ASSERT(is_file_pattern("/pattern"));
    CPPUNIT_ASSERT(is_file_pattern("*/pattern"));
}


void UtilsPatternsTest::test_is_glob_pattern() {
    CPPUNIT_ASSERT(!is_glob_pattern(""));
    CPPUNIT_ASSERT(!is_glob_pattern("no_glob_pattern"));
    CPPUNIT_ASSERT(is_glob_pattern("glob*_pattern"));
    CPPUNIT_ASSERT(is_glob_pattern("glob[sdf]_pattern"));
    CPPUNIT_ASSERT(is_glob_pattern("glob?_pattern"));
}


void UtilsPatternsTest::test_is_cmdline_package_spec() {
    CPPUNIT_ASSERT(!is_cmdline_package_spec(""));
    CPPUNIT_ASSERT(!is_cmdline_package_spec("pkg"));
    CPPUNIT_ASSERT(!is_cmdline_package_spec(".rpm"));
    CPPUNIT_ASSERT(!is_cmdline_package_spec("gopher://example.com/pkg"));
    CPPUNIT_ASSERT(is_cmdline_package_spec("gopher://example.com/pkg.rpm"));
    CPPUNIT_ASSERT(is_cmdline_package_spec("/repository/pkg.rpm"));
    CPPUNIT_ASSERT(is_cmdline_package_spec("file:///repository/pkg.rpm"));
    CPPUNIT_ASSERT(is_cmdline_package_spec("FILE:///repository/pkg.rpm"));
    CPPUNIT_ASSERT(is_cmdline_package_spec("http://example.com/pkg?marker.rpm"));
    CPPUNIT_ASSERT(is_cmdline_package_spec("https://example.com/pkg"));
    CPPUNIT_ASSERT(is_cmdline_package_spec("ftp://example.com/pkg"));
}
