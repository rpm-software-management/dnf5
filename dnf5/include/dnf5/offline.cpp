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

#include <dnf5/offline.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/fs/file.hpp>

namespace dnf5::offline {

OfflineTransactionState::OfflineTransactionState(std::filesystem::path path) : path(std::move(path)) {
    read();
}
void OfflineTransactionState::read() {
    try {
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

std::filesystem::path get_offline_datadir() {
    return DEFAULT_DATADIR;
}

}  // namespace dnf5::offline
