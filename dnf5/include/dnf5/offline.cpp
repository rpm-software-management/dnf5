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

#include <dnf5/offline.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/fs/file.hpp>

#ifdef WITH_SYSTEMD
#include <systemd/sd-journal.h>
#endif

namespace dnf5::offline {

OfflineTransactionState::OfflineTransactionState(std::filesystem::path path) : path(std::move(path)) {
    read();
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

void log_status(
    Context & context,
    const std::string & message,
    [[maybe_unused]] const std::string & message_id,
    [[maybe_unused]] const std::string & system_releasever,
    [[maybe_unused]] const std::string & target_releasever) {
    const auto & version = get_application_version();
    const std::string & version_string = fmt::format("{}.{}.{}", version.major, version.minor, version.micro);

    auto logger = context.base.get_logger();
    logger->info(message);

#ifdef WITH_SYSTEMD
    sd_journal_send(
        "MESSAGE=%s",
        message.c_str(),
        "MESSAGE_ID=%s",
        message_id.c_str(),
        "SYSTEM_RELEASEVER=%s",
        system_releasever.c_str(),
        "TARGET_RELEASEVER=%s",
        target_releasever.c_str(),
        "DNF_VERSION=%s",
        version_string.c_str(),
        NULL);
#endif
}

}  // namespace dnf5::offline
