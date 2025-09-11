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

#ifndef DNF5DAEMON_SERVER_SERVICES_OFFLINE_OFFLINE_HPP
#define DNF5DAEMON_SERVER_SERVICES_OFFLINE_OFFLINE_HPP

#include "session.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <filesystem>

class Offline : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~Offline() = default;
    void dbus_register();
    void dbus_deregister();

private:
    sdbus::MethodReply impl_cancel(sdbus::MethodCall & call, const dnfdaemon::KeyValueMap & options);
    sdbus::MethodReply cancel_with_options(sdbus::MethodCall & call);
    sdbus::MethodReply cancel(sdbus::MethodCall & call);
    sdbus::MethodReply impl_clean(sdbus::MethodCall & call, const dnfdaemon::KeyValueMap & options);
    sdbus::MethodReply clean_with_options(sdbus::MethodCall & call);
    sdbus::MethodReply clean(sdbus::MethodCall & call);
    sdbus::MethodReply get_status(sdbus::MethodCall & call);
    sdbus::MethodReply impl_set_finish_action(
        sdbus::MethodCall & call, const std::string & action, const dnfdaemon::KeyValueMap & options);
    sdbus::MethodReply set_finish_action_with_options(sdbus::MethodCall & call);
    sdbus::MethodReply set_finish_action(sdbus::MethodCall & call);

    enum class Scheduled { NOT_SCHEDULED, ANOTHER_TOOL, SCHEDULED };
    Scheduled offline_transaction_scheduled();
    std::filesystem::path get_datadir();
    std::filesystem::path get_destdir();
};

#endif
