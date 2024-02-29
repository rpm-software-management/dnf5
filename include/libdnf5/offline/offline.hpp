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

#ifndef LIBDNF5_OFFLINE_HPP
#define LIBDNF5_OFFLINE_HPP

#include <dnf5/context.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <libdnf5/conf/option_number.hpp>
#include <sdbus-c++/sdbus-c++.h>
#include <toml.hpp>

namespace libdnf5::offline {

const std::string OFFLINE_STARTED_ID{"3e0a5636d16b4ca4bbe5321d06c6aa62"};
const std::string OFFLINE_FINISHED_ID{"8cec00a1566f4d3594f116450395f06c"};

const std::string STATUS_DOWNLOAD_INCOMPLETE{"download-incomplete"};
const std::string STATUS_DOWNLOAD_COMPLETE{"download-complete"};
const std::string STATUS_READY{"ready"};
const std::string STATUS_UPGRADE_INCOMPLETE{"upgrade-incomplete"};

const int STATE_VERSION = 0;
const std::string STATE_HEADER{"offline-transaction-state"};

const std::filesystem::path DEFAULT_DATADIR{std::filesystem::path(libdnf5::SYSTEM_STATE_DIR) / "offline"};
const std::filesystem::path TRANSACTION_STATE_FILENAME{"offline-transaction-state.toml"};
const std::filesystem::path TRANSACTION_JSON_FILENAME{"offline-transaction.json"};

std::filesystem::path get_offline_datadir(const std::filesystem::path & installroot);

struct OfflineTransactionStateData {
    int state_version = STATE_VERSION;
    std::string status = STATUS_DOWNLOAD_INCOMPLETE;
    std::string cachedir;
    std::string target_releasever;
    std::string system_releasever;
    std::string verb;
    std::string cmd_line;
    bool poweroff_after = false;
    std::vector<std::string> enabled_repos;
    std::vector<std::string> disabled_repos;
};

class OfflineTransactionState {
public:
    void write();
    OfflineTransactionState(std::filesystem::path path);
    OfflineTransactionStateData & get_data() { return data; };
    const std::exception_ptr & get_read_exception() const { return read_exception; };

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
    enabled_repos,
    disabled_repos)

#endif  // LIBDNF5_OFFLINE_HPP
