// Copyright Contributors to the DNF5 project.
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


#include "test_fs.hpp"

#include "utils/fs/utils.hpp"

#include <fcntl.h>
#include <libdnf5/common/exception.hpp>
#include <unistd.h>

#include <filesystem>


using namespace libdnf5::utils::fs;
namespace stdfs = std::filesystem;


CPPUNIT_TEST_SUITE_REGISTRATION(UtilsFsTest);


// Generates a null-terminated string "abcdefghijabcdef..." of given size
static void generate_test_data(char * buffer, std::size_t size) {
    for (std::size_t i = 0; i < size - 1; ++i) {
        buffer[i] = 'a' + static_cast<char>(i % 10);
    }
    buffer[size - 1] = '\0';
}


static std::string generate_test_data(std::size_t size) {
    std::string res;
    for (std::size_t i = 0; i < size / 10; ++i) {
        res.append("abcdefghij");
    }

    res.append(std::string("abcdefghij", size % 10));
    return res;
}


void UtilsFsTest::test_temp_dir() {
    stdfs::path path;

    {
        libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_temp_dir");
        path = temp_dir.get_path();

        CPPUNIT_ASSERT(path.native().starts_with("/tmp/libdnf_unittest_temp_dir."));
        CPPUNIT_ASSERT(stdfs::exists(path));
        CPPUNIT_ASSERT_EQUAL(stdfs::status(path).type(), stdfs::file_type::directory);
    }
    CPPUNIT_ASSERT(!stdfs::exists(path));

    // test creating temp dir at a custom location (in another temp dir) and removing a non-empty dir
    {
        libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_temp_dir");
        libdnf5::utils::fs::TempDir nested_temp_dir(temp_dir.get_path(), "nested_temp_dir");
        path = temp_dir.get_path();

        CPPUNIT_ASSERT(nested_temp_dir.get_path().native().starts_with((path / "nested_temp_dir.").native()));
        CPPUNIT_ASSERT(stdfs::exists(nested_temp_dir.get_path()));
        CPPUNIT_ASSERT_EQUAL(stdfs::status(nested_temp_dir.get_path()).type(), stdfs::file_type::directory);
    }
    CPPUNIT_ASSERT(!stdfs::exists(path));
}


void UtilsFsTest::test_temp_file_creation() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_temp_file_creation");

    stdfs::path path;
    {
        libdnf5::utils::fs::TempFile temp_file(temp_dir.get_path() / "temp_file");
        path = temp_file.get_path();

        CPPUNIT_ASSERT(path.native().starts_with((temp_dir.get_path() / "temp_file.").native()));
        CPPUNIT_ASSERT(stdfs::exists(path));
        CPPUNIT_ASSERT_EQUAL(stdfs::status(path).type(), stdfs::file_type::regular);
    }
    CPPUNIT_ASSERT(!stdfs::exists(path));
}


void UtilsFsTest::test_temp_file_operation() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_temp_file_operation");

    stdfs::path path;
    {
        libdnf5::utils::fs::TempFile temp_file(temp_dir.get_path() / "temp.file");

        path = temp_file.get_path();

        CPPUNIT_ASSERT_GREATER(-1, temp_file.get_fd());

        char buf[8] = {};
        CPPUNIT_ASSERT_EQUAL((ssize_t)6, write(temp_file.get_fd(), "hello", 6));
        CPPUNIT_ASSERT_EQUAL((ssize_t)0, read(temp_file.get_fd(), buf, 6));
        CPPUNIT_ASSERT_EQUAL((off_t)0, lseek(temp_file.get_fd(), 0, SEEK_SET));
        CPPUNIT_ASSERT_EQUAL((ssize_t)6, read(temp_file.get_fd(), buf, 8));
        CPPUNIT_ASSERT_EQUAL(std::string("hello"), std::string(buf));

        auto & file = temp_file.open_as_file("w+");
        CPPUNIT_ASSERT(static_cast<bool>(file));
        CPPUNIT_ASSERT(file.get_path() == temp_file.get_path());

        temp_file.close();
        CPPUNIT_ASSERT_EQUAL(-1, temp_file.get_fd());
        CPPUNIT_ASSERT(!temp_file.get_file());
    }
    CPPUNIT_ASSERT(!stdfs::exists(path));
}


void UtilsFsTest::test_temp_file_release() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_temp_file_release");

    stdfs::path path;
    {
        libdnf5::utils::fs::TempFile temp_file(temp_dir.get_path() / "temp.file");

        path = temp_file.get_path();

        temp_file.release();
        CPPUNIT_ASSERT_EQUAL(-1, temp_file.get_fd());
        CPPUNIT_ASSERT(!temp_file.get_file());
        CPPUNIT_ASSERT_EQUAL(std::string(), temp_file.get_path().native());
    }
    CPPUNIT_ASSERT(stdfs::exists(path));
}


void UtilsFsTest::test_file_basic() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_file_basic");

    libdnf5::utils::fs::File file(temp_dir.get_path() / "file", "w+");
    auto path = file.get_path();

    CPPUNIT_ASSERT_EQUAL(temp_dir.get_path() / "file", path);
    CPPUNIT_ASSERT(file.get() != nullptr);
    CPPUNIT_ASSERT_GREATEREQUAL(0, file.get_fd());
    CPPUNIT_ASSERT(static_cast<bool>(file));

    file.close();

    CPPUNIT_ASSERT_EQUAL(stdfs::path(), file.get_path());
    CPPUNIT_ASSERT_EQUAL(static_cast<FILE *>(nullptr), file.get());
    CPPUNIT_ASSERT_THROW(file.get_fd(), libdnf5::AssertionError);
    CPPUNIT_ASSERT(!static_cast<bool>(file));

    CPPUNIT_ASSERT(stdfs::exists(path));
    CPPUNIT_ASSERT_EQUAL(stdfs::status(path).type(), stdfs::file_type::regular);

    libdnf5::utils::fs::File file2;
    CPPUNIT_ASSERT_THROW(file2.read(), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(file2.tell(), libdnf5::AssertionError);
}


void UtilsFsTest::test_file_simple_io() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_file_simple_io");

    libdnf5::utils::fs::File file(temp_dir.get_path() / "file", "w");
    auto path = file.get_path();

    char data_w[100];
    generate_test_data(data_w, sizeof(data_w));

    file.write(data_w, sizeof(data_w));
    file.close();

    char data_r[150] = {};

    // Read less than the size of the file
    file.open(path, "r");
    std::size_t res = file.read(data_r, 50);
    CPPUNIT_ASSERT_EQUAL((size_t)50, res);  // in theory, the read() could read less and this would fail
    CPPUNIT_ASSERT_EQUAL(std::string(data_w, 50), std::string(data_r, 50));
    CPPUNIT_ASSERT(!file.is_at_eof());
    file.close();
    file.open(path, "r");

    // Attempt to read more than the size of the file
    res = file.read(data_r, 150);
    CPPUNIT_ASSERT_EQUAL((size_t)100, res);  // in theory, the read() could read less and this would fail
    CPPUNIT_ASSERT_EQUAL(std::string(data_w, 100), std::string(data_r, 100));
    CPPUNIT_ASSERT(file.is_at_eof());
}


void UtilsFsTest::test_file_open_fd() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_file_open_fd");

    auto file_path = temp_dir.get_path() / "file";
    {
        int fd = open(file_path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
        CPPUNIT_ASSERT_GREATEREQUAL(0, fd);

        libdnf5::utils::fs::File file(fd, file_path, "w");
        file.write("hello");
    }

    {
        int fd = open(file_path.c_str(), O_RDONLY);
        CPPUNIT_ASSERT_GREATEREQUAL(0, fd);

        libdnf5::utils::fs::File file;
        file.open(fd, file_path, "r");
        auto data = file.read();
        CPPUNIT_ASSERT_EQUAL(std::string("hello"), data);
    }
}


void UtilsFsTest::test_file_putc_getc() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_file_putc_getc");

    libdnf5::utils::fs::File file(temp_dir.get_path() / "file", "w");
    auto path = file.get_path();

    file.putc('a');
    file.write("bc");
    file.putc('d');
    file.close();

    file.open(path, "r");
    char c;
    CPPUNIT_ASSERT_EQUAL(true, file.getc(c));
    CPPUNIT_ASSERT_EQUAL('a', c);
    CPPUNIT_ASSERT_EQUAL(true, file.getc(c));
    CPPUNIT_ASSERT_EQUAL('b', c);
    file.seek(-1, SEEK_END);
    CPPUNIT_ASSERT_EQUAL(true, file.getc(c));
    CPPUNIT_ASSERT_EQUAL('d', c);
    CPPUNIT_ASSERT_EQUAL(false, file.getc(c));
}


void UtilsFsTest::test_file_high_level_io() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_file_buffered_io");

    // simple write / read data
    {
        libdnf5::utils::fs::File file(temp_dir.get_path() / "file3", "w");
        auto path = file.get_path();

        std::string data_w = generate_test_data(100);

        file.write(data_w);
        file.close();

        file.open(path, "r");
        std::string data_r = file.read();
        CPPUNIT_ASSERT_EQUAL(data_w, data_r);
    }

    // write / read big data
    {
        libdnf5::utils::fs::File file(temp_dir.get_path() / "file2", "w");
        auto path = file.get_path();

        std::string data_w = generate_test_data(1e6);  // 1MB of data

        file.write(data_w);
        file.close();

        file.open(path, "r");
        std::string data_r = file.read();
        CPPUNIT_ASSERT_EQUAL(data_w, data_r);
    }

    // read from the middle, read certain amount of chars
    {
        libdnf5::utils::fs::File file(temp_dir.get_path() / "file3", "w");
        auto path = file.get_path();

        std::string data_w = generate_test_data(200);

        file.write(data_w);
        file.close();

        file.open(path, "r");
        file.seek(100, SEEK_SET);
        std::string data_r = file.read();
        CPPUNIT_ASSERT_EQUAL(data_w.substr(100), data_r);

        file.seek(100, SEEK_SET);
        data_r = file.read(50);
        CPPUNIT_ASSERT_EQUAL(data_w.substr(100, 50), data_r);

        file.seek(100, SEEK_SET);
        data_r = file.read(150);
        CPPUNIT_ASSERT_EQUAL(data_w.substr(100), data_r);
    }
}


void UtilsFsTest::test_file_read_line() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_file_read_line");

    libdnf5::utils::fs::File file(temp_dir.get_path() / "file", "w");
    auto path = file.get_path();

    std::string data_w;
    data_w.append("line 1\n");
    data_w.append("\n");
    data_w.append("line 3\n");
    data_w.append("\n");

    file.write(data_w);
    file.close();

    file.open(path, "r");
    std::string data_r;
    CPPUNIT_ASSERT_EQUAL(true, file.read_line(data_r));
    CPPUNIT_ASSERT_EQUAL(std::string("line 1"), data_r);
    CPPUNIT_ASSERT_EQUAL(true, file.read_line(data_r));
    CPPUNIT_ASSERT_EQUAL(std::string(), data_r);
    CPPUNIT_ASSERT_EQUAL(true, file.read_line(data_r));
    CPPUNIT_ASSERT_EQUAL(std::string("line 3"), data_r);
    CPPUNIT_ASSERT_EQUAL(true, file.read_line(data_r));
    CPPUNIT_ASSERT_EQUAL(std::string(), data_r);
    CPPUNIT_ASSERT_EQUAL(false, file.read_line(data_r));
}


void UtilsFsTest::test_file_seek() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_file_release");

    std::string data_w = generate_test_data(10);

    libdnf5::utils::fs::File file(temp_dir.get_path() / "file", "w+");
    file.write(data_w);

    CPPUNIT_ASSERT_EQUAL(10l, file.tell());

    file.seek(4, SEEK_SET);
    CPPUNIT_ASSERT_EQUAL(4l, file.tell());
    CPPUNIT_ASSERT_EQUAL(data_w.substr(4), file.read());

    file.seek(4, SEEK_SET);
    file.seek(1, SEEK_CUR);
    CPPUNIT_ASSERT_EQUAL(5l, file.tell());
    CPPUNIT_ASSERT_EQUAL(data_w.substr(5), file.read());

    file.seek(4, SEEK_SET);
    file.seek(-1, SEEK_CUR);
    CPPUNIT_ASSERT_EQUAL(3l, file.tell());
    CPPUNIT_ASSERT_EQUAL(data_w.substr(3), file.read());

    file.seek(-1, SEEK_END);
    CPPUNIT_ASSERT_EQUAL(9l, file.tell());
    CPPUNIT_ASSERT_EQUAL(data_w.substr(9), file.read());

    CPPUNIT_ASSERT_THROW(file.seek(-1, SEEK_SET), libdnf5::FileSystemError);
}


void UtilsFsTest::test_file_release() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_file_release");

    std::string data_w = generate_test_data(10);
    FILE * file_p = nullptr;

    {
        libdnf5::utils::fs::File file(temp_dir.get_path() / "file", "w+");

        file.write(data_w);
        file.rewind();
        file_p = file.release();
    }

    // make sure we can still read from the released FILE *
    char data_r[10] = {};
    std::size_t res = std::fread(data_r, sizeof(char), sizeof(data_r), file_p);
    CPPUNIT_ASSERT_EQUAL((size_t)10, res);  // in theory, the read() could read less and this would fail
    CPPUNIT_ASSERT_EQUAL(data_w, std::string(data_r, sizeof(data_r)));
    CPPUNIT_ASSERT_EQUAL(0, std::fclose(file_p));
}


void UtilsFsTest::test_file_flush() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_unittest_file_release");

    std::string data_w = generate_test_data(10);

    libdnf5::utils::fs::File file_w(temp_dir.get_path() / "file", "w+");

    file_w.write(data_w);
    file_w.flush();

    libdnf5::utils::fs::File file_r(temp_dir.get_path() / "file", "r");
    auto data_r = file_r.read();

    CPPUNIT_ASSERT_EQUAL(data_w, data_r);
}
