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

#include "libdnf5/conf/const.hpp"
#include "libdnf5/defs.h"

#include <filesystem>
#include <memory>

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

// This value comes from systemd, see
// https://www.freedesktop.org/wiki/Software/systemd/SystemUpdates or
// systemd.offline-updates(7).
const std::filesystem::path MAGIC_SYMLINK{"/system-update"};

const std::filesystem::path DEFAULT_DATADIR{std::filesystem::path(libdnf5::SYSTEM_STATE_DIR) / "offline"};
const std::filesystem::path TRANSACTION_STATE_FILENAME{"offline-transaction-state.toml"};

class OfflineTransactionState;

/// Data of the initiated offline transaction state, by default stored in the
/// /usr/lib/sysimage/libdnf5/offline/offline-transaction-state.toml file.
struct LIBDNF_API OfflineTransactionStateData {
public:
    friend OfflineTransactionState;

    OfflineTransactionStateData();
    ~OfflineTransactionStateData();

    OfflineTransactionStateData(const OfflineTransactionStateData & src);
    OfflineTransactionStateData & operator=(const OfflineTransactionStateData & src);

    OfflineTransactionStateData(OfflineTransactionStateData && src) noexcept;
    OfflineTransactionStateData & operator=(OfflineTransactionStateData && src) noexcept;

    /// Set the transaction state data file version
    void set_state_version(int state_version);
    int get_state_version() const;

    /// Set current offline transaction status. One of download-incomplete,
    /// download-complete, ready, or transaction-incomplete.
    void set_status(const std::string & status);
    const std::string & get_status() const;

    /// Set the cachedir to be used for the offline transaction.
    void set_cachedir(const std::string & cachedir);
    const std::string & get_cachedir() const;

    /// Set the target releasever for the offline transaction.
    void set_target_releasever(const std::string & target_releasever);
    const std::string & get_target_releasever() const;

    /// Set the detected releasever in time the offline transaction was initialized.
    void set_system_releasever(const std::string & system_releasever);
    const std::string & get_system_releasever() const;

    /// Set the dnf command used to initialize the offline transaction (e.g. "system-upgrade download").
    void set_verb(const std::string & verb);
    const std::string & get_verb() const;

    /// Set the command line used to initialize the offline transaction.
    void set_cmd_line(const std::string & cmd_line);
    const std::string & get_cmd_line() const;

    /// Set whether the system power off after the operation is complete is required
    void set_poweroff_after(bool poweroff_after);
    bool get_poweroff_after() const;

    /// Set module_platform_id for the offline transaction.
    void set_module_platform_id(const std::string & module_platform_id);
    const std::string & get_module_platform_id() const;

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};


/// Class to handle offline transaction state.
class LIBDNF_API OfflineTransactionState {
public:
    OfflineTransactionState() = delete;
    ~OfflineTransactionState();

    /// Constructs a new OfflineTransactionState instance based on the state file location.
    /// @param path Path to the state file (default location is /usr/lib/sysimage/libdnf5/offline/offline-transaction-state.toml).
    OfflineTransactionState(std::filesystem::path path);

    OfflineTransactionState(const OfflineTransactionState & src);
    OfflineTransactionState & operator=(const OfflineTransactionState & src);
    OfflineTransactionState(OfflineTransactionState && src) noexcept;
    OfflineTransactionState & operator=(OfflineTransactionState && src) noexcept;

    /// Returns offline transaction state data.
    OfflineTransactionStateData & get_data();
    /// Write the current state to the file.
    void write();
    /// Returns any exception caught during the reading of the state file (or nullptr if no exception occurred).
    const std::exception_ptr & get_read_exception() const;
    /// Returns path to the state file.
    std::filesystem::path get_path() const;

private:
    class LIBDNF_LOCAL Impl;
    /// Read offline transaction state data from the file
    LIBDNF_LOCAL void read();
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::offline

#endif  // LIBDNF5_TRANSACTION_OFFLINE_HPP
