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

#ifndef DNF5DAEMON_SERVER_SERVICES_BASE_BASE_HPP
#define DNF5DAEMON_SERVER_SERVICES_BASE_BASE_HPP

#include "session.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <string>
#include <vector>

class Base : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~Base() = default;
    void dbus_register();
    void dbus_deregister();

private:
    sdbus::MethodReply impl_clean(
        sdbus::MethodCall & call, const std::string & cache_type, const dnfdaemon::KeyValueMap & options);
    sdbus::MethodReply clean_with_options(sdbus::MethodCall & call);
    sdbus::MethodReply clean(sdbus::MethodCall & call);
    sdbus::MethodReply reset(sdbus::MethodCall & call);
    sdbus::MethodReply read_all_repos(sdbus::MethodCall & call);
};

#endif
