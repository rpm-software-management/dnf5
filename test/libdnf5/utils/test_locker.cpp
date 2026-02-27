// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: GPL-2.0-or-later


#include "test_locker.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/locker.hpp"

#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <string>


using namespace libdnf5::utils;

CPPUNIT_TEST_SUITE_REGISTRATION(UtilsLockerTest);


void UtilsLockerTest::setUp() {
    temp_dir = std::filesystem::temp_directory_path() / "libdnf5_locker_test";
    std::filesystem::create_directories(temp_dir);
    lock_file_path = temp_dir + "/test.lock";
}


void UtilsLockerTest::tearDown() {
    std::filesystem::remove_all(temp_dir);
}


void UtilsLockerTest::test_constructor() {
    Locker locker(lock_file_path);
}


void UtilsLockerTest::test_read_lock_success() {
    Locker locker(lock_file_path);
    CPPUNIT_ASSERT(locker.read_lock());
    CPPUNIT_ASSERT(std::filesystem::exists(lock_file_path));
}


void UtilsLockerTest::test_write_lock_success() {
    Locker locker(lock_file_path);
    CPPUNIT_ASSERT(locker.write_lock());
    CPPUNIT_ASSERT(std::filesystem::exists(lock_file_path));
}


void UtilsLockerTest::test_write_content() {
    Locker locker(lock_file_path);
    CPPUNIT_ASSERT(locker.write_lock());

    const std::string test_content = "test content\nline 2\n";
    locker.write_content(test_content);

    std::ifstream file(lock_file_path);
    std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    CPPUNIT_ASSERT_EQUAL(test_content, file_content);
}


void UtilsLockerTest::test_read_content() {
    {
        std::ofstream file(lock_file_path);
        file << "existing content\nwith multiple lines\n";
    }

    Locker locker(lock_file_path);
    CPPUNIT_ASSERT(locker.read_lock());

    std::string content = locker.read_content();
    CPPUNIT_ASSERT_EQUAL(std::string("existing content\nwith multiple lines\n"), content);
}


void UtilsLockerTest::test_read_write_content_cycle() {
    Locker locker(lock_file_path);
    CPPUNIT_ASSERT(locker.write_lock());

    const std::string initial_content = "initial content";
    locker.write_content(initial_content);

    std::string read_content = locker.read_content();
    CPPUNIT_ASSERT_EQUAL(initial_content, read_content);

    const std::string updated_content = "updated content\nwith new line";
    locker.write_content(updated_content);

    read_content = locker.read_content();
    CPPUNIT_ASSERT_EQUAL(updated_content, read_content);
}


void UtilsLockerTest::test_unlock() {
    Locker locker(lock_file_path);
    CPPUNIT_ASSERT(locker.write_lock());
    CPPUNIT_ASSERT(std::filesystem::exists(lock_file_path));

    locker.unlock();
    CPPUNIT_ASSERT(!std::filesystem::exists(lock_file_path));

    Locker locker2(lock_file_path);
    CPPUNIT_ASSERT(locker2.write_lock());
}


void UtilsLockerTest::test_destructor_cleanup() {
    {
        Locker locker(lock_file_path);
        CPPUNIT_ASSERT(locker.write_lock());
        CPPUNIT_ASSERT(std::filesystem::exists(lock_file_path));
    }

    CPPUNIT_ASSERT(!std::filesystem::exists(lock_file_path));

    Locker locker2(lock_file_path);
    CPPUNIT_ASSERT(locker2.write_lock());
}


void UtilsLockerTest::test_nonexistent_directory() {
    std::string invalid_path = "/nonexistent/directory/test.lock";
    Locker locker(invalid_path);

    CPPUNIT_ASSERT_THROW(locker.write_lock(), libdnf5::SystemError);
    CPPUNIT_ASSERT_THROW(locker.read_lock(), libdnf5::SystemError);
}
