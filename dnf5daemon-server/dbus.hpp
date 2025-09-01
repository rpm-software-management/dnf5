// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5DAEMON_SERVER_DBUS_HPP
#define DNF5DAEMON_SERVER_DBUS_HPP

#include <libdnf5/sdbus_compat.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <map>
#include <string>

namespace dnfdaemon {

// types
using KeyValueMap = std::map<std::string, sdbus::Variant>;
using KeyValueMapList = std::vector<KeyValueMap>;
enum class RepoStatus { NOT_READY, PENDING, READY, ERROR };
enum class ResolveResult { NO_PROBLEM, WARNING, ERROR };

enum class DbusTransactionItemType : int { PACKAGE, GROUP, ENVIRONMENT, MODULE, SKIPPED };

using DbusTransactionItem = sdbus::Struct<
    std::string,   // DbusTransactionItemType
    std::string,   // libdnf5::transaction::TransactionItemAction
    std::string,   // libdnf5::transaction::TransactionItemReason
    KeyValueMap,   // other transaction item attributes - e.g. group id for REASON_CHANGE to GROUP,
                   // packages that are replaced by this transaction item
    KeyValueMap>;  // transaction object (package / group / module)

// (timestamp, author, text)
using Changelog = sdbus::Struct<int64_t, std::string, std::string>;

// (id, type, title, url)
using AdvisoryReference = sdbus::Struct<std::string, std::string, std::string, std::string>;

// constants

const SDBUS_SERVICE_NAME_TYPE DBUS_NAME{"org.rpm.dnf.v0"};
const sdbus::ObjectPath DBUS_OBJECT_PATH{"/org/rpm/dnf/v0"};

// interfaces
const SDBUS_INTERFACE_NAME_TYPE INTERFACE_BASE{"org.rpm.dnf.v0.Base"};
const SDBUS_INTERFACE_NAME_TYPE INTERFACE_REPO{"org.rpm.dnf.v0.rpm.Repo"};
const SDBUS_INTERFACE_NAME_TYPE INTERFACE_RPM{"org.rpm.dnf.v0.rpm.Rpm"};
const SDBUS_INTERFACE_NAME_TYPE INTERFACE_GOAL{"org.rpm.dnf.v0.Goal"};
const SDBUS_INTERFACE_NAME_TYPE INTERFACE_GROUP{"org.rpm.dnf.v0.comps.Group"};
const SDBUS_INTERFACE_NAME_TYPE INTERFACE_HISTORY{"org.rpm.dnf.v0.History"};
const SDBUS_INTERFACE_NAME_TYPE INTERFACE_ADVISORY{"org.rpm.dnf.v0.Advisory"};
const SDBUS_INTERFACE_NAME_TYPE INTERFACE_OFFLINE{"org.rpm.dnf.v0.Offline"};
const SDBUS_INTERFACE_NAME_TYPE INTERFACE_SESSION_MANAGER{"org.rpm.dnf.v0.SessionManager"};

// signals
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_DOWNLOAD_ADD_NEW{"download_add_new"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_DOWNLOAD_PROGRESS{"download_progress"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_DOWNLOAD_END{"download_end"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_DOWNLOAD_MIRROR_FAILURE{"download_mirror_failure"};

const SDBUS_SIGNAL_NAME_TYPE SIGNAL_REPO_KEY_IMPORT_REQUEST{"repo_key_import_request"};

const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_BEFORE_BEGIN{"transaction_before_begin"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_AFTER_COMPLETE{"transaction_after_complete"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_TRANSACTION_START{"transaction_transaction_start"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_TRANSACTION_PROGRESS{"transaction_transaction_progress"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_TRANSACTION_STOP{"transaction_transaction_stop"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_VERIFY_START{"transaction_verify_start"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_VERIFY_PROGRESS{"transaction_verify_progress"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_VERIFY_STOP{"transaction_verify_stop"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_ACTION_START{"transaction_action_start"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_ACTION_PROGRESS{"transaction_action_progress"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_ACTION_STOP{"transaction_action_stop"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_SCRIPT_START{"transaction_script_start"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_SCRIPT_STOP{"transaction_script_stop"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_SCRIPT_ERROR{"transaction_script_error"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_UNPACK_ERROR{"transaction_unpack_error"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_ELEM_PROGRESS{"transaction_elem_progress"};
const SDBUS_SIGNAL_NAME_TYPE SIGNAL_TRANSACTION_FINISHED{"transaction_finished"};

const SDBUS_SIGNAL_NAME_TYPE SIGNAL_WRITE_TO_FD_FINISHED{"write_to_fd_finished"};

// polkit actions
const char * const POLKIT_REPOCONF_WRITE = "org.rpm.dnf.v0.rpm.Repo.conf_write";
const char * const POLKIT_EXECUTE_RPM_TRANSACTION = "org.rpm.dnf.v0.rpm.execute_transaction";
const char * const POLKIT_EXECUTE_RPM_TRUSTED_TRANSACTION = "org.rpm.dnf.v0.rpm.execute_trusted_transaction";
const char * const POLKIT_CONFIRM_KEY_IMPORT = "org.rpm.dnf.v0.rpm.Repo.confirm_key";
const char * const POLKIT_CONFIG_OVERRIDE = "org.rpm.dnf.v0.base.Config.override";

// errors
const SDBUS_ERROR_NAME_TYPE ERROR{"org.rpm.dnf.v0.Error"};
const SDBUS_ERROR_NAME_TYPE ERROR_REPOCONF{"org.rpm.dnf.v0.rpm.Repo.ConfError"};
const SDBUS_ERROR_NAME_TYPE ERROR_REPO_ID_UNKNOWN{"org.rpm.dnf.v0.rpm.Repo.NoMatchingIdError"};
const SDBUS_ERROR_NAME_TYPE ERROR_RESOLVE{"org.rpm.dnf.v0.rpm.Rpm.ResolveError"};
const SDBUS_ERROR_NAME_TYPE ERROR_TRANSACTION{"org.rpm.dnf.v0.rpm.Rpm.TransactionError"};

}  // namespace dnfdaemon

#endif
