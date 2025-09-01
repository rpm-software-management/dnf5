// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "libdnf5/utils/fs/file.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-lib.h"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

extern "C" {
#include <solv/solv_xfopen.h>
}

#include <stdio.h>

#include <algorithm>

#define libdnf_assert_file_open() libdnf_assert(file != nullptr, "The operation requires an open file");


namespace libdnf5::utils::fs {

File::File() = default;


File::File(const std::filesystem::path & path, const char * mode, bool use_solv_xfopen) {
    open(path, mode, use_solv_xfopen);
}


File::File(int fd, const std::filesystem::path & path, const char * mode, bool use_solv_xfopen_fd) {
    open(fd, path, mode, use_solv_xfopen_fd);
}


File::File(File && f) noexcept : path(std::move(f.path)), file(f.file) {
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
        throw FileSystemError(errno, path, M_("cannot open file"));
    }

    this->path = path;
}


void File::open(int fd, const std::filesystem::path & path, const char * mode, bool use_solv_xfopen_fd) {
    close();

    file = use_solv_xfopen_fd ? solv_xfopen_fd(path.c_str(), fd, mode) : ::fdopen(fd, mode);
    if (file == nullptr) {
        this->path = "";
        throw FileSystemError(errno, path, M_("cannot open file from fd"));
    }

    this->path = path;
}


void File::close() {
    if (file != nullptr) {
        if (std::fclose(file) != 0) {
            throw FileSystemError(errno, path, M_("cannot close file"));
        }
    }

    file = nullptr;
    path.clear();
}


std::FILE * File::release() noexcept {
    auto * p = file;
    file = nullptr;
    path.clear();
    return p;
}


std::size_t File::read(void * buffer, std::size_t count) {
    libdnf_assert_file_open();

    auto res = std::fread(buffer, sizeof(char), count, file);

    if (res != count) {
        if (std::feof(file) != 0) {
            return res;
        }

        libdnf_assert(
            std::ferror(file) != 0, "Failed to read \"{}\", error expected but no error detected", path.native());

        throw FileSystemError(errno, path, M_("error reading file"));
    }

    return res;
}


void File::write(const void * buffer, std::size_t count) {
    libdnf_assert_file_open();

    if (std::fwrite(buffer, sizeof(char), count, file) != count) {
        libdnf_assert(std::feof(file) == 0, "EOF reached while writing file \"{}\"", path.native());
        libdnf_assert(
            std::ferror(file) > 0, "Failed to write \"{}\", error expected but no error detected", path.native());

        throw FileSystemError(errno, path, M_("error writing file"));
    }
}


bool File::getc(char & c) {
    libdnf_assert_file_open();

    int res;
    if ((res = std::fgetc(file)) == EOF) {
        if (is_at_eof()) {
            return false;
        }

        throw FileSystemError(errno, path, M_("error reading file"));
    }

    c = static_cast<char>(res);
    return true;
}


void File::putc(char c) {
    libdnf_assert_file_open();

    if (std::fputc(c, file) == EOF) {
        throw FileSystemError(errno, path, M_("error writing file"));
    }
}


void File::flush() {
    libdnf_assert_file_open();

    if (std::fflush(file) == -1) {
        throw FileSystemError(errno, path, M_("error flushing file"));
    }
}


void File::seek(long offset, int whence) {
    libdnf_assert_file_open();

    if (std::fseek(file, offset, whence) == -1) {
        throw FileSystemError(errno, path, M_("error seeking in file"));
    }
}


long File::tell() const {
    libdnf_assert_file_open();

    auto res = std::ftell(file);

    if (res == -1) {
        throw FileSystemError(errno, path, M_("error retrieving file position"));
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
    libdnf_assert_file_open();

    // Try to detect the length to the end of the file.
    std::size_t length_to_end;
    bool length_detected{false};
    if (auto cur_pos = std::ftell(file); cur_pos != -1) {
        if (std::fseek(file, 0, SEEK_END) != -1) {
            if (auto end_pos = std::ftell(file); end_pos != -1) {
                length_to_end = static_cast<std::size_t>(end_pos - cur_pos);
                length_detected = true;
            }
        }
        std::fseek(file, cur_pos, SEEK_SET);
    }

    std::string res;

    if (length_detected) {
        // The file length is known. Allocate memory at once and read data.
        std::size_t to_read = count == 0 ? length_to_end : std::min(length_to_end, count);
        res.resize(to_read);
        std::size_t size = read(res.data(), to_read);
        libdnf_assert(size == to_read, "Short read occurred: expected to read {}, have read {}.", to_read, size);
    } else {
        // Could not determine file length. A fallback solution, we read data in blocks and reallocate memory.
        char buffer[4096];
        if (count > 0) {
            // We read at most `count` characters.
            do {
                std::size_t to_read = std::min(sizeof buffer, count);
                auto chars_read = read(buffer, to_read);
                res.append(buffer, chars_read);
                if (chars_read < to_read) {
                    break;
                }
                count -= chars_read;
            } while (count > 0);
        } else {
            // We are reading the file to the end.
            do {
                auto chars_read = read(buffer, sizeof buffer);
                res.append(buffer, chars_read);
                if (chars_read < sizeof buffer) {
                    break;
                }
            } while (true);
        }
    }

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

        throw FileSystemError(errno, path, M_("error reading a line from file"));
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


File::operator bool() const noexcept {
    return file != nullptr;
}


const std::filesystem::path & File::get_path() const noexcept {
    return path;
}


std::FILE * File::get() const noexcept {
    return file;
}


int File::get_fd() const {
    libdnf_assert_file_open();

    int fd = ::fileno(file);

    if (fd == -1) {
        throw FileSystemError(errno, path, M_("error retrieving file descriptor"));
    }

    return fd;
}

}  // namespace libdnf5::utils::fs
