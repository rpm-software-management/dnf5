/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-server: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-server is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-server is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-server.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNFDAEMON_SERVER_DBUS_HPP
#define DNFDAEMON_SERVER_DBUS_HPP

#include <sdbus-c++/sdbus-c++.h>

#include <map>
#include <string>

namespace dnfdaemon {

// types
using KeyValueMap = std::map<std::string, sdbus::Variant>;
using KeyValueMapList = std::vector<KeyValueMap>;
enum class RepoStatus { NOT_READY, PENDING, READY, ERROR };

using DbusTransactionItem = sdbus::Struct<
    unsigned int,  // action
    std::string,   // name
    std::string,   // epoch
    std::string,   // version
    std::string,   // release
    std::string,   // arch
    std::string    // repoid
    >;


// constants

const char * const DBUS_NAME = "org.rpm.dnf.v0";
const char * const DBUS_OBJECT_PATH = "/org/rpm/dnf/v0";

// interfaces
const char * const INTERFACE_BASE = "org.rpm.dnf.v0.Base";
const char * const INTERFACE_REPO = "org.rpm.dnf.v0.rpm.Repo";
const char * const INTERFACE_REPOCONF = "org.rpm.dnf.v0.rpm.RepoConf";
const char * const INTERFACE_RPM = "org.rpm.dnf.v0.rpm.Rpm";
const char * const INTERFACE_SESSION_MANAGER = "org.rpm.dnf.v0.SessionManager";

// signals
const char * const SIGNAL_REPO_LOAD_START = "repo_load_start";
const char * const SIGNAL_REPO_LOAD_PROGRESS = "repo_load_progress";
const char * const SIGNAL_REPO_LOAD_END = "repo_load_end";

const char * const SIGNAL_PACKAGE_DOWNLOAD_START = "package_download_start";
const char * const SIGNAL_PACKAGE_DOWNLOAD_PROGRESS = "package_download_progress";
const char * const SIGNAL_PACKAGE_DOWNLOAD_END = "package_download_end";
const char * const SIGNAL_PACKAGE_DOWNLOAD_MIRROR_FAILURE = "package_download_mirror_failure";

const char * const SIGNAL_TRANSACTION_TRANSACTION_START = "transaction_transaction_start";
const char * const SIGNAL_TRANSACTION_TRANSACTION_PROGRESS = "transaction_transaction_progress";
const char * const SIGNAL_TRANSACTION_TRANSACTION_STOP = "transaction_transaction_stop";
const char * const SIGNAL_TRANSACTION_VERIFY_START = "transaction_verify_start";
const char * const SIGNAL_TRANSACTION_VERIFY_PROGRESS = "transaction_verify_progress";
const char * const SIGNAL_TRANSACTION_VERIFY_STOP = "transaction_verify_stop";
const char * const SIGNAL_TRANSACTION_ACTION_START = "transaction_action_start";
const char * const SIGNAL_TRANSACTION_ACTION_PROGRESS = "transaction_action_progress";
const char * const SIGNAL_TRANSACTION_ACTION_STOP = "transaction_action_stop";
const char * const SIGNAL_TRANSACTION_SCRIPT_START = "transaction_script_start";
const char * const SIGNAL_TRANSACTION_SCRIPT_STOP = "transaction_script_stop";
const char * const SIGNAL_TRANSACTION_SCRIPT_ERROR = "transaction_script_error";
const char * const SIGNAL_TRANSACTION_UNPACK_ERROR = "transaction_unpack_error";
const char * const SIGNAL_TRANSACTION_ELEM_PROGRESS = "transaction_elem_progress";

// polkit actions
const char * const POLKIT_REPOCONF_WRITE = "org.rpm.dnf.v0.rpm.RepoConf.write";
const char * const POLKIT_EXECUTE_RPM_TRANSACTION = "org.rpm.dnf.v0.rpm.execute_transaction";

// errors
const char * const ERROR = "org.rpm.dnf.v0.Error";
const char * const ERROR_REPOCONF = "org.rpm.dnf.v0.rpm.RepoConf.Error";
const char * const ERROR_RESOLVE = "org.rpm.dnf.v0.rpm.Rpm.ResolveError";
const char * const ERROR_TRANSACTION = "org.rpm.dnf.v0.rpm.Rpm.TransactionError";

}  // namespace dnfdaemon

#define DNFDAEMON_ERROR_REPLY(_CALL, _EXCEPTION)                                     \
    auto _reply = (_CALL).createErrorReply({dnfdaemon::ERROR, (_EXCEPTION).what()}); \
    try {                                                                            \
        _reply.send();                                                               \
    } catch (...) {                                                                  \
    }                                                                                \
    /**/

#endif
