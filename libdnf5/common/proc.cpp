// Copyright Contributors to the DNF5 project.
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

#include "libdnf5/common/proc.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fmt/format.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

namespace libdnf5 {

uid_t read_login_uid_from_proc(pid_t pid) noexcept {
    auto in = open(fmt::format("/proc/{}/loginuid", pid).c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (in == -1) {
        return INVALID_UID;
    }

    ssize_t len;
    char buf[16];
    do {
        errno = 0;
        len = read(in, buf, sizeof(buf) - 1);
    } while (len < 0 && errno == EINTR);

    close(in);

    if (len <= 0) {
        return INVALID_UID;
    }

    buf[len] = '\0';
    char * endptr;
    errno = 0;
    auto uid = static_cast<uid_t>(strtol(buf, &endptr, 10));
    if (buf == endptr || errno != 0) {
        return INVALID_UID;
    }

    return uid;
}

uid_t get_login_uid() noexcept {
    static uid_t cached_uid = libdnf5::INVALID_UID;
    if (cached_uid == libdnf5::INVALID_UID) {
        cached_uid = libdnf5::read_login_uid_from_proc(getpid());
        if (cached_uid == libdnf5::INVALID_UID) {
            cached_uid = getuid();
        }
    }
    return cached_uid;
}

std::set<pid_t> fuser(const std::filesystem::path & path) {
    std::set<pid_t> pids;
    std::error_code ec;

    struct stat target_st;
    if (stat(path.c_str(), &target_st) == -1) {
        throw libdnf5::SystemError{errno, M_("Cannot stat {}"), path.string()};
    }

    for (const auto & entry : std::filesystem::directory_iterator{"/proc", ec}) {
        if (ec) {
            continue;
        }

        const std::string & entry_name = entry.path().filename().string();
        if (!entry.is_directory(ec) || !std::all_of(entry_name.begin(), entry_name.end(), isdigit)) {
            continue;
        }
        const pid_t pid = std::stoi(entry_name);

        const std::filesystem::path fd_dir_path = entry.path() / "fd";
        for (const auto & fd_entry : std::filesystem::directory_iterator{fd_dir_path, ec}) {
            if (ec) {
                continue;
            }
            struct stat fd_st;
            if (stat(fd_entry.path().c_str(), &fd_st) != -1) {
                if (fd_st.st_dev == target_st.st_dev && fd_st.st_ino == target_st.st_ino) {
                    pids.insert(pid);
                    break;
                }
            }
        }
    }

    return pids;
}

std::vector<std::string> pid_cmdline(const pid_t pid) {
    const std::filesystem::path path{libdnf5::utils::sformat("/proc/{}/cmdline", pid)};
    std::ifstream stream{path, std::ios::binary};
    if (!stream.is_open()) {
        throw libdnf5::SystemError{errno, M_("Cannot open {}"), path.string()};
    }

    std::vector<std::string> args;
    std::string current;
    char c;
    while (stream.get(c)) {
        if (c == '\0') {
            args.push_back(current);
            current = "";
        } else {
            current += c;
        }
    }
    args.push_back(current);
    return args;
}

}  // namespace libdnf5
