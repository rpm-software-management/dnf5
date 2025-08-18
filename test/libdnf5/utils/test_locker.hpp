// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LIBDNF5_TEST_UTILS_LOCKER_HPP
#define LIBDNF5_TEST_UTILS_LOCKER_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class UtilsLockerTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(UtilsLockerTest);
    CPPUNIT_TEST(test_constructor);
    CPPUNIT_TEST(test_read_lock_success);
    CPPUNIT_TEST(test_write_lock_success);
    CPPUNIT_TEST(test_write_content);
    CPPUNIT_TEST(test_read_content);
    CPPUNIT_TEST(test_read_write_content_cycle);
    CPPUNIT_TEST(test_unlock);
    CPPUNIT_TEST(test_destructor_cleanup);
    CPPUNIT_TEST(test_nonexistent_directory);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_constructor();
    void test_read_lock_success();
    void test_write_lock_success();
    void test_write_content();
    void test_read_content();
    void test_read_write_content_cycle();
    void test_unlock();
    void test_destructor_cleanup();
    void test_nonexistent_directory();

private:
    std::string temp_dir;
    std::string lock_file_path;
};


#endif  // LIBDNF5_TEST_UTILS_LOCKER_HPP
