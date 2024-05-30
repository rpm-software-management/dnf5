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

#ifndef LIBDNF5_TRANSACTION_OFFLINE_HPP
#define LIBDNF5_TRANSACTION_OFFLINE_HPP

#include <libdnf5/conf/const.hpp>
#include <toml.hpp>

#include <filesystem>

namespace libdnf5::offline {

// Unique identifiers used to mark and identify system-upgrade boots in
// journald logs. These are the same as they are in `dnf4 system-upgrade`, so
// `dnf5 offline log` will find offline transactions performed by DNF 4 and
// vice-versa.
const std::string REBOOT_REQUESTED_ID{"9348174c5cc74001a71ef26bd79d302e"};
const std::string OFFLINE_STARTED_ID{"3e0a5636d16b4ca4bbe5321d06c6aa62"};
const std::string OFFLINE_FINISHED_ID{"8cec00a1566f4d3594f116450395f06c"};

const std::string STATUS_DOWNLOAD_INCOMPLETE{"download-incomplete"};
const std::string STATUS_DOWNLOAD_COMPLETE{"download-complete"};
const std::string STATUS_READY{"ready"};
const std::string STATUS_TRANSACTION_INCOMPLETE{"transaction-incomplete"};

const int STATE_VERSION = 1;
const std::string STATE_HEADER{"offline-transaction-state"};

const std::filesystem::path DEFAULT_DATADIR{std::filesystem::path(libdnf5::SYSTEM_STATE_DIR) / "offline"};
const std::filesystem::path TRANSACTION_STATE_FILENAME{"offline-transaction-state.toml"};

struct OfflineTransactionStateData {
    int state_version = STATE_VERSION;
    std::string status = STATUS_DOWNLOAD_INCOMPLETE;
    std::string cachedir;
    std::string target_releasever;
    std::string system_releasever;
    std::string verb;
    std::string cmd_line;
    bool poweroff_after = false;
    std::string module_platform_id;
};

class OfflineTransactionState {
public:
    void write();
    OfflineTransactionState(std::filesystem::path path);
    OfflineTransactionStateData & get_data();
    const std::exception_ptr & get_read_exception() const;
    std::filesystem::path get_path() const;

private:
    void read();
    std::exception_ptr read_exception;
    std::filesystem::path path;
    OfflineTransactionStateData data;
};

}  // namespace libdnf5::offline

TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(
    libdnf5::offline::OfflineTransactionStateData,
    state_version,
    status,
    cachedir,
    target_releasever,
    system_releasever,
    verb,
    cmd_line,
    poweroff_after,
    module_platform_id)

#endif  // LIBDNF5_TRANSACTION_OFFLINE_HPP
