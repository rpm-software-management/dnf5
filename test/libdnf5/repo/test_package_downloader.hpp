// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_REPO_PACKAGE_DOWNLOADER_HPP
#define LIBDNF5_TEST_REPO_PACKAGE_DOWNLOADER_HPP

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
