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

#include "test_temp_files_memory.hpp"

#include "../shared/utils.hpp"
#include "repo/temp_files_memory.hpp"

#include "libdnf/common/exception.hpp"

#include <fmt/format.h>


CPPUNIT_TEST_SUITE_REGISTRATION(TempFilesMemoryTest);

using namespace libdnf::repo;


void TempFilesMemoryTest::setUp() {
    CppUnit::TestCase::setUp();
    temp_dir = std::make_unique<libdnf::utils::fs::TempDir>("libdnf_test_filesmemory");
    parent_dir_path = temp_dir->get_path();
    full_path = parent_dir_path / TempFilesMemory::MEMORY_FILENAME;
}

void TempFilesMemoryTest::tearDown() {
    temp_dir.reset();
    CppUnit::TestCase::tearDown();
}

void TempFilesMemoryTest::test_directory_is_created_when_not_exists() {
    auto test_path = temp_dir->get_path() / "some/new/path";
    CPPUNIT_ASSERT(!std::filesystem::exists(test_path));
    TempFilesMemory memory(test_path);
    CPPUNIT_ASSERT(std::filesystem::exists(test_path));
}

void TempFilesMemoryTest::test_get_files_when_empty_storage() {
    TempFilesMemory memory(temp_dir->get_path() / "unknown/path");
    CPPUNIT_ASSERT(memory.get_files().empty());
}

void TempFilesMemoryTest::test_get_files_throws_exception_when_invalid_format() {
    libdnf::utils::fs::File(full_path, "w").write("");
    TempFilesMemory memory_empty(parent_dir_path);
    CPPUNIT_ASSERT_THROW(memory_empty.get_files(), libdnf::Error);

    libdnf::utils::fs::File(full_path, "w").write("[\"path1\", \"path2\", \"path3\"]");
    TempFilesMemory memory_invalid(parent_dir_path);
    CPPUNIT_ASSERT_THROW(memory_invalid.get_files(), libdnf::Error);
}

void TempFilesMemoryTest::test_get_files_returns_stored_values() {
    libdnf::utils::fs::File(full_path, "w")
        .write(fmt::format(
            "{} = [\"path/to/package1.rpm\", \"different/path/to/package2.rpm\"]",
            TempFilesMemory::FILE_PATHS_TOML_KEY));
    TempFilesMemory memory(parent_dir_path);
    std::vector<std::string> expected = {"path/to/package1.rpm", "different/path/to/package2.rpm"};
    CPPUNIT_ASSERT_EQUAL(expected, memory.get_files());
}

void TempFilesMemoryTest::test_add_files_when_empty_storage() {
    std::vector<std::string> new_paths = {"path1", "path2"};

    TempFilesMemory memory(parent_dir_path);
    CPPUNIT_ASSERT(memory.get_files().empty());

    memory.add_files(new_paths);
    CPPUNIT_ASSERT_EQUAL(new_paths, memory.get_files());
}

void TempFilesMemoryTest::test_add_files_when_existing_storage() {
    std::vector<std::string> new_paths = {"path3", "path4"};
    libdnf::utils::fs::File(full_path, "w")
        .write(fmt::format("{} = [\"path1\", \"path2\"]", TempFilesMemory::FILE_PATHS_TOML_KEY));

    TempFilesMemory memory(parent_dir_path);
    memory.add_files(new_paths);
    std::vector<std::string> expected = {"path1", "path2", "path3", "path4"};
    CPPUNIT_ASSERT_EQUAL(expected, memory.get_files());
}

void TempFilesMemoryTest::test_add_files_deduplicates_and_sorts_data() {
    std::vector<std::string> new_paths = {"path1", "path2", "path4", "path1"};
    libdnf::utils::fs::File(full_path, "w")
        .write(fmt::format("{} = [\"path4\", \"path1\", \"path4\", \"path3\"]", TempFilesMemory::FILE_PATHS_TOML_KEY));

    TempFilesMemory memory(parent_dir_path);
    memory.add_files(new_paths);
    std::vector<std::string> expected = {"path1", "path2", "path3", "path4"};
    CPPUNIT_ASSERT_EQUAL(expected, memory.get_files());
}

void TempFilesMemoryTest::test_clear_deletes_storage_content() {
    libdnf::utils::fs::File(full_path, "w")
        .write(fmt::format(
            "{} = [\"/path/to/package1.rpm\", \"/different-path/to/package2.rpm\", "
            "\"/another-path/leading/to/pkg3.rpm\"]",
            TempFilesMemory::FILE_PATHS_TOML_KEY));
    TempFilesMemory memory(parent_dir_path);
    std::vector<std::string> expected = {
        "/path/to/package1.rpm", "/different-path/to/package2.rpm", "/another-path/leading/to/pkg3.rpm"};
    CPPUNIT_ASSERT_EQUAL(expected, memory.get_files());

    memory.clear();
    CPPUNIT_ASSERT(memory.get_files().empty());
}

void TempFilesMemoryTest::test_clear_when_empty_storage() {
    TempFilesMemory memory(temp_dir->get_path() / "non-existing/path");
    memory.clear();
}
