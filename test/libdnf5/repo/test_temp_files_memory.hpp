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

#ifndef LIBDNF5_TEST_REPO_TEMP_FILES_MEMORY_HPP
#define LIBDNF5_TEST_REPO_TEMP_FILES_MEMORY_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>

#include <filesystem>


class TempFilesMemoryTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(TempFilesMemoryTest);
    CPPUNIT_TEST(test_directory_is_created_when_not_exists);
    CPPUNIT_TEST(test_get_files_when_empty_storage);
    CPPUNIT_TEST(test_get_files_throws_exception_when_invalid_format);
    CPPUNIT_TEST(test_get_files_returns_empty_vector_when_missing_key);
    CPPUNIT_TEST(test_get_files_returns_stored_values);
    CPPUNIT_TEST(test_add_files_when_empty_storage);
    CPPUNIT_TEST(test_add_no_files_when_empty_storage);
    CPPUNIT_TEST(test_add_files_when_existing_storage);
    CPPUNIT_TEST(test_add_files_deduplicates_and_sorts_data);
    CPPUNIT_TEST(test_clear_deletes_storage_content);
    CPPUNIT_TEST(test_clear_when_empty_storage);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_directory_is_created_when_not_exists();
    void test_get_files_when_empty_storage();
    void test_get_files_throws_exception_when_invalid_format();
    void test_get_files_returns_empty_vector_when_missing_key();
    void test_get_files_returns_stored_values();
    void test_add_files_when_empty_storage();
    void test_add_no_files_when_empty_storage();
    void test_add_files_when_existing_storage();
    void test_add_files_deduplicates_and_sorts_data();
    void test_clear_deletes_storage_content();
    void test_clear_when_empty_storage();

private:
    std::filesystem::path parent_dir_path;
    std::filesystem::path full_path;
};

#endif
