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


#include "test_fs.hpp"

#include "utils/fs/utils.hpp"

#include <filesystem>


using namespace libdnf::utils::fs;
namespace stdfs = std::filesystem;


CPPUNIT_TEST_SUITE_REGISTRATION(UtilsFsTest);


void UtilsFsTest::test_temp_dir() {
    stdfs::path path;

    {
        libdnf::utils::fs::TempDir temp_dir("libdnf_unittest_temp_dir");
        path = temp_dir.get_path();

        CPPUNIT_ASSERT(path.native().starts_with("/tmp/libdnf_unittest_temp_dir."));
        CPPUNIT_ASSERT(stdfs::exists(path));
        CPPUNIT_ASSERT_EQUAL(stdfs::status(path).type(), stdfs::file_type::directory);
    }
    CPPUNIT_ASSERT(!stdfs::exists(path));

    // test creating temp dir at a custom location (in another temp dir) and removing a non-empty dir
    {
        libdnf::utils::fs::TempDir temp_dir("libdnf_unittest_temp_dir");
        libdnf::utils::fs::TempDir nested_temp_dir(temp_dir.get_path(), "nested_temp_dir");
        path = temp_dir.get_path();

        CPPUNIT_ASSERT(nested_temp_dir.get_path().native().starts_with((path / "nested_temp_dir.").native()));
        CPPUNIT_ASSERT(stdfs::exists(nested_temp_dir.get_path()));
        CPPUNIT_ASSERT_EQUAL(stdfs::status(nested_temp_dir.get_path()).type(), stdfs::file_type::directory);
    }
    CPPUNIT_ASSERT(!stdfs::exists(path));
}
