// Copyright Contributors to the DNF5 project.
// Copyright (C) 2024 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "dnf5/offline.hpp"

#ifdef WITH_SYSTEMD
#include <systemd/sd-journal.h>
#endif

namespace dnf5::offline {

void log_status(
    Context & context,
    const std::string & message,
    [[maybe_unused]] const std::string & message_id,
    [[maybe_unused]] const std::string & system_releasever,
    [[maybe_unused]] const std::string & target_releasever) {
    const auto & version = get_application_version();
    const std::string & version_string = fmt::format("{}.{}.{}", version.major, version.minor, version.micro);

    auto logger = context.get_base().get_logger();
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
