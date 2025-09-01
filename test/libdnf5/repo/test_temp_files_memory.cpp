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

#include "test_temp_files_memory.hpp"

#include "../shared/utils.hpp"
#include "repo/temp_files_memory.hpp"

#include <fmt/format.h>
#include <libdnf5/common/exception.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(TempFilesMemoryTest);

using namespace libdnf5::repo;


void TempFilesMemoryTest::setUp() {
    BaseTestCase::setUp();
    parent_dir_path = temp_dir->get_path();
    full_path = parent_dir_path / TempFilesMemory::MEMORY_FILENAME;
}

void TempFilesMemoryTest::test_directory_is_created_when_not_exists() {
    auto test_path = parent_dir_path / "some/new/path";
    CPPUNIT_ASSERT(!std::filesystem::exists(test_path));
    TempFilesMemory memory(base.get_weak_ptr(), test_path);
    CPPUNIT_ASSERT(std::filesystem::exists(test_path));
}

void TempFilesMemoryTest::test_get_files_when_empty_storage() {
    TempFilesMemory memory(base.get_weak_ptr(), parent_dir_path / "unknown/path");
    CPPUNIT_ASSERT(memory.get_files().empty());
}

void TempFilesMemoryTest::test_get_files_returns_empty_vector_when_missing_key() {
    libdnf5::utils::fs::File(full_path, "w").write("");
    TempFilesMemory memory_empty(base.get_weak_ptr(), parent_dir_path);
    CPPUNIT_ASSERT(memory_empty.get_files().empty());

    libdnf5::utils::fs::File(full_path, "w").write("UNKNOWN_KEY = [\"path1\", \"path2\", \"path3\"]");
    TempFilesMemory memory_unknown_key(base.get_weak_ptr(), parent_dir_path);
    CPPUNIT_ASSERT(memory_unknown_key.get_files().empty());
}

void TempFilesMemoryTest::test_get_files_throws_exception_when_invalid_format() {
    libdnf5::utils::fs::File(full_path, "w").write("[\"path1\", \"path2\", \"path3\"]");
    TempFilesMemory memory_invalid(base.get_weak_ptr(), parent_dir_path);
    CPPUNIT_ASSERT_THROW(memory_invalid.get_files(), libdnf5::Error);
}

void TempFilesMemoryTest::test_get_files_returns_stored_values() {
    libdnf5::utils::fs::File(full_path, "w")
        .write(fmt::format(
            "{} = [\"path/to/package1.rpm\", \"different/path/to/package2.rpm\"]",
            TempFilesMemory::FILE_PATHS_TOML_KEY));
    TempFilesMemory memory(base.get_weak_ptr(), parent_dir_path);
    std::vector<std::string> expected = {"path/to/package1.rpm", "different/path/to/package2.rpm"};
    CPPUNIT_ASSERT_EQUAL(expected, memory.get_files());
}

void TempFilesMemoryTest::test_add_files_when_empty_storage() {
    std::vector<std::string> new_paths = {"path1", "path2"};

    TempFilesMemory memory(base.get_weak_ptr(), parent_dir_path);
    CPPUNIT_ASSERT(memory.get_files().empty());

    memory.add_files(new_paths);
    CPPUNIT_ASSERT_EQUAL(new_paths, memory.get_files());
}

void TempFilesMemoryTest::test_add_no_files_when_empty_storage() {
    // make sure that adding an empty vector will not cause crashing

    TempFilesMemory memory(base.get_weak_ptr(), parent_dir_path);
    CPPUNIT_ASSERT(memory.get_files().empty());

    std::vector<std::string> empty_paths = {};
    memory.add_files(empty_paths);
    CPPUNIT_ASSERT(memory.get_files().empty());
}

void TempFilesMemoryTest::test_add_files_when_existing_storage() {
    std::vector<std::string> new_paths = {"path3", "path4"};
    libdnf5::utils::fs::File(full_path, "w")
        .write(fmt::format("{} = [\"path1\", \"path2\"]", TempFilesMemory::FILE_PATHS_TOML_KEY));

    TempFilesMemory memory(base.get_weak_ptr(), parent_dir_path);
    memory.add_files(new_paths);
    std::vector<std::string> expected = {"path1", "path2", "path3", "path4"};
    CPPUNIT_ASSERT_EQUAL(expected, memory.get_files());
}

void TempFilesMemoryTest::test_add_files_deduplicates_and_sorts_data() {
    std::vector<std::string> new_paths = {"path1", "path2", "path4", "path1"};
    libdnf5::utils::fs::File(full_path, "w")
        .write(fmt::format("{} = [\"path4\", \"path1\", \"path4\", \"path3\"]", TempFilesMemory::FILE_PATHS_TOML_KEY));

    TempFilesMemory memory(base.get_weak_ptr(), parent_dir_path);
    memory.add_files(new_paths);
    std::vector<std::string> expected = {"path1", "path2", "path3", "path4"};
    CPPUNIT_ASSERT_EQUAL(expected, memory.get_files());
}

void TempFilesMemoryTest::test_clear_deletes_storage_content() {
    libdnf5::utils::fs::File(full_path, "w")
        .write(fmt::format(
            "{} = [\"/path/to/package1.rpm\", \"/different-path/to/package2.rpm\", "
            "\"/another-path/leading/to/pkg3.rpm\"]",
            TempFilesMemory::FILE_PATHS_TOML_KEY));
    TempFilesMemory memory(base.get_weak_ptr(), parent_dir_path);
    std::vector<std::string> expected = {
        "/path/to/package1.rpm", "/different-path/to/package2.rpm", "/another-path/leading/to/pkg3.rpm"};
    CPPUNIT_ASSERT_EQUAL(expected, memory.get_files());

    memory.clear();
    CPPUNIT_ASSERT(memory.get_files().empty());
}

void TempFilesMemoryTest::test_clear_when_empty_storage() {
    TempFilesMemory memory(base.get_weak_ptr(), parent_dir_path / "non-existing/path");
    memory.clear();
}
