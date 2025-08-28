// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_REPO_FILE_DOWNLOADER_HPP
#define LIBDNF5_TEST_REPO_FILE_DOWNLOADER_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class FileDownloaderTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(FileDownloaderTest);
    CPPUNIT_TEST(test_file_downloader);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_file_downloader();
};

#endif
