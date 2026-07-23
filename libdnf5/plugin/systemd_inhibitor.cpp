// Copyright Contributors to the DNF5 project.
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

#include "libdnf5/plugin/systemd_inhibitor.hpp"

#include "systemd_inhibitor_private.hpp"

#include <unistd.h>

#include <mutex>
#include <unordered_map>

namespace libdnf5::plugin {

namespace {

// Keyed by the address of the libdnf5::base::Transaction object passed to a
// plugin's pre_transaction()/post_transaction() hooks - the same object for
// both calls during one transaction run.
std::mutex g_systemd_inhibitor_fds_mutex;
std::unordered_map<const void *, int> g_systemd_inhibitor_fds;

}  // namespace

void register_systemd_inhibitor_fd(const libdnf5::base::Transaction & transaction, int fd) {
    if (fd < 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_systemd_inhibitor_fds_mutex);
    auto [it, inserted] = g_systemd_inhibitor_fds.try_emplace(&transaction, fd);
    if (!inserted) {
        // Guard against a caller registering twice - avoid leaking the first fd.
        close(it->second);
        it->second = fd;
    }
}

void close_systemd_inhibitor_fd(const libdnf5::base::Transaction & transaction) noexcept {
    std::lock_guard<std::mutex> lock(g_systemd_inhibitor_fds_mutex);
    auto it = g_systemd_inhibitor_fds.find(&transaction);
    if (it != g_systemd_inhibitor_fds.end()) {
        close(it->second);
        g_systemd_inhibitor_fds.erase(it);
    }
}

}  // namespace libdnf5::plugin
