/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNF5DAEMON_SERVER_SERVICES_REPO_REPO_HPP
#define DNF5DAEMON_SERVER_SERVICES_REPO_REPO_HPP

#include "session.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <string>
#include <vector>

class Repo : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~Repo() = default;
    void dbus_register();
    void dbus_deregister();

private:
    sdbus::MethodReply list(sdbus::MethodCall & call);
    sdbus::MethodReply impl_confirm_key(
        sdbus::MethodCall & call, const std::string & key_id, bool confirmed, const dnfdaemon::KeyValueMap & options);
    sdbus::MethodReply confirm_key_with_options(sdbus::MethodCall & call);
    sdbus::MethodReply confirm_key(sdbus::MethodCall & call);
    sdbus::MethodReply impl_enable_disable(
        sdbus::MethodCall & call,
        bool enable,
        const std::vector<std::string> & ids,
        const dnfdaemon::KeyValueMap & options);
    sdbus::MethodReply enable_with_options(sdbus::MethodCall & call);
    sdbus::MethodReply enable(sdbus::MethodCall & call);
    sdbus::MethodReply disable_with_options(sdbus::MethodCall & call);
    sdbus::MethodReply disable(sdbus::MethodCall & call);

    void enable_disable_repos(const std::vector<std::string> & ids, const bool enable);
};

#endif
