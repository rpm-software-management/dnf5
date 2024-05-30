/*
Copyright (C) 2024 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf5/common/exception.hpp"

#include <libdnf5/transaction/offline.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/fs/file.hpp>


namespace libdnf5::offline {

OfflineTransactionState::OfflineTransactionState(std::filesystem::path path) : path(std::move(path)) {
    read();
}

OfflineTransactionStateData & OfflineTransactionState::get_data() {
    return data;
}

const std::exception_ptr & OfflineTransactionState::get_read_exception() const {
    return read_exception;
}

std::filesystem::path OfflineTransactionState::get_path() const {
    return path;
}


void OfflineTransactionState::read() {
    try {
        const std::ifstream file{path};
        if (!file.good()) {
            throw libdnf5::FileSystemError(errno, path, M_("error reading offline state file"));
        }
        const auto & value = toml::parse(path);
        data = toml::find<OfflineTransactionStateData>(value, STATE_HEADER);
        if (data.state_version != STATE_VERSION) {
            throw libdnf5::RuntimeError(M_("incompatible version of state data"));
        }
    } catch (const std::exception & ex) {
        read_exception = std::current_exception();
        data = OfflineTransactionStateData{};
    }
}

void OfflineTransactionState::write() {
    auto file = libdnf5::utils::fs::File(path, "w");
    file.write(toml::format(toml::value{{STATE_HEADER, data}}));
    file.close();
}

}  // namespace libdnf5::offline
