/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "file.hpp"

#include "libdnf/common/exception.hpp"

extern "C" {
#include <solv/solv_xfopen.h>
}

#include <cstdio>


#define libdnf_assert_file_open() libdnf_assert(file != nullptr, "The operation requires an open file");


namespace libdnf::utils::fs {

File::File(const std::filesystem::path & path, const char * mode, bool use_solv_xfopen) {
    open(path, mode, use_solv_xfopen);
}


File::File(int fd, const std::filesystem::path & path, const char * mode, bool use_solv_xfopen_fd) {
    open(fd, path, mode, use_solv_xfopen_fd);
}


File::File(File && f) : path(std::move(f.path)), file(f.file) {
    f.file = nullptr;
}


File & File::operator=(File && f) {
    if (&f != this) {
        close();
        path = std::move(f.path);
        file = f.file;
        f.file = nullptr;
    }

    return *this;
}


File::~File() {
    try {
        close();
    } catch (std::exception &) {
        // TODO(lukash) consider logging or printing the exception
    }
}


void File::open(const std::filesystem::path & path, const char * mode, bool use_solv_xfopen) {
    close();

    file = use_solv_xfopen ? solv_xfopen(path.c_str(), mode) : std::fopen(path.c_str(), mode);

    if (file == nullptr) {
        this->path = "";
        throw std::filesystem::filesystem_error(
            "cannot open file", path, std::error_code(errno, std::system_category()));
    }

    this->path = path;
}


void File::open(int fd, const std::filesystem::path & path, const char * mode, bool use_solv_xfopen_fd) {
    close();

    file = use_solv_xfopen_fd ? solv_xfopen_fd(path.c_str(), fd, mode) : ::fdopen(fd, mode);
    if (file == nullptr) {
        this->path = "";
        throw std::filesystem::filesystem_error(
            "cannot open file from fd", path, std::error_code(errno, std::system_category()));
    }

    this->path = path;
}


void File::close() {
    if (file != nullptr) {
        if (std::fclose(file) != 0) {
            throw std::filesystem::filesystem_error(
                "cannot close file", path, std::error_code(errno, std::system_category()));
        }
    }

    file = nullptr;
    path.clear();
}


FILE * File::release() noexcept {
    FILE * p = file;
    file = nullptr;
    path.clear();
    return p;
}


std::size_t File::read(void * buffer, std::size_t count) {
    libdnf_assert_file_open();

    auto res = std::fread(buffer, sizeof(char), count, file);

    if (res != count) {
        if (std::feof(file)) {
            return res;
        }

        libdnf_assert(
            std::ferror(file) > 0, "Failed to read \"{}\", error expected but no error detected", path.native());

        throw std::filesystem::filesystem_error(
            "error reading file", path, std::error_code(errno, std::system_category()));
    }

    return res;
}


void File::write(const void * buffer, std::size_t count) {
    libdnf_assert_file_open();

    if (std::fwrite(buffer, sizeof(char), count, file) != count) {
        libdnf_assert(std::feof(file) == 0, "EOF reached while writing file \"{}\"", path.native());
        libdnf_assert(
            std::ferror(file) > 0, "Failed to write \"{}\", error expected but no error detected", path.native());

        throw std::filesystem::filesystem_error(
            "error writing file", path, std::error_code(errno, std::system_category()));
    }
}


bool File::getc(char & c) {
    libdnf_assert_file_open();

    int res;
    if ((res = std::fgetc(file)) == EOF) {
        if (is_at_eof()) {
            return false;
        }

        throw std::filesystem::filesystem_error(
            "error reading file", path, std::error_code(errno, std::system_category()));
    }

    c = static_cast<char>(res);
    return true;
}


void File::putc(char c) {
    libdnf_assert_file_open();

    if (std::fputc(c, file) == EOF) {
        throw std::filesystem::filesystem_error(
            "error writing file", path, std::error_code(errno, std::system_category()));
    }
}


void File::flush() {
    libdnf_assert_file_open();

    if (std::fflush(file) == -1) {
        throw std::filesystem::filesystem_error(
            "error flushing file", path, std::error_code(errno, std::system_category()));
    }
}


void File::seek(long offset, int whence) {
    libdnf_assert_file_open();

    if (std::fseek(file, offset, whence) == -1) {
        throw std::filesystem::filesystem_error(
            "error seeking in file", path, std::error_code(errno, std::system_category()));
    }
}


long File::tell() const {
    libdnf_assert_file_open();

    auto res = std::ftell(file);

    if (res == -1) {
        throw std::filesystem::filesystem_error(
            "error retrieving file position", path, std::error_code(errno, std::system_category()));
    }

    return res;
}


void File::rewind() {
    libdnf_assert_file_open();
    std::rewind(file);
}


bool File::is_at_eof() const {
    libdnf_assert_file_open();
    return std::feof(file);
}


std::string File::read(std::size_t count) {
    long cur_pos = tell();
    seek(0, SEEK_END);
    std::size_t length_till_end = static_cast<std::size_t>(tell() - cur_pos);
    seek(cur_pos, SEEK_SET);

    std::size_t to_read = count == 0 ? length_till_end : std::min(length_till_end, count);
    std::string res;
    res.resize(to_read);

    std::size_t size = read(res.data(), to_read);
    libdnf_assert(size == to_read, "Short read occurred: expected to read {}, have read {}.", to_read, size);

    return res;
}


bool File::read_line(std::string & line) {
    libdnf_assert_file_open();

    char * buf = nullptr;
    std::size_t buf_len = 0;
    ssize_t line_len = 0;
    if ((line_len = getline(&buf, &buf_len, file)) == -1) {
        if (errno != ENOMEM) {
            free(buf);
        }

        if (is_at_eof()) {
            return false;
        }

        throw std::filesystem::filesystem_error(
            "error reading a line from file", path, std::error_code(errno, std::system_category()));
    }

    while (line_len > 0 && (buf[line_len - 1] == '\r' || buf[line_len - 1] == '\n')) {
        --line_len;
    }
    line.assign(buf, static_cast<std::size_t>(line_len));
    free(buf);
    return true;
}


void File::write(std::string_view data) {
    write(data.data(), data.size());
}


int File::get_fd() const {
    libdnf_assert_file_open();

    int fd = ::fileno(file);

    if (fd == -1) {
        throw std::filesystem::filesystem_error(
            "error retrieving file descriptor", path, std::error_code(errno, std::system_category()));
    }

    return fd;
}

}  // namespace libdnf::utils::fs
