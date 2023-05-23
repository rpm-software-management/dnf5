/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_TEST_REPO_PACKAGE_DOWNLOADER_HPP
#define LIBDNF_TEST_REPO_PACKAGE_DOWNLOADER_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class PackageDownloaderTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(PackageDownloaderTest);
    CPPUNIT_TEST(test_package_downloader);
    CPPUNIT_TEST(test_package_downloader_temp_files_memory);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_package_downloader();
    void test_package_downloader_temp_files_memory();
};

#endif
