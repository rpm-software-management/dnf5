// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LIBDNF5_TEST_FILE_LOGGER_HPP
#define LIBDNF5_TEST_FILE_LOGGER_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class FileLoggerTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(FileLoggerTest);
    CPPUNIT_TEST(test_file_logger_add);
    CPPUNIT_TEST(test_file_logger_create_name);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;
    void test_file_logger_create_name();
    void test_file_logger_add();

private:
    std::filesystem::path full_log_path;
};

#endif
