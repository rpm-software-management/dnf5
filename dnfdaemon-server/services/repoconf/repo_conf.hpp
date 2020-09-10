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

#ifndef DNFDAEMON_SERVER_SERVICES_REPOCONF_HPP
#define DNFDAEMON_SERVER_SERVICES_REPOCONF_HPP

#include "dnfdaemon-server/dbus.hpp"
#include "dnfdaemon-server/session.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <string>
#include <vector>

class RepoConf : public IDbusSessionService {
public:
    using IDbusSessionService::IDbusSessionService;
    ~RepoConf() = default;
    void dbus_register();
    void dbus_deregister();

private:
    std::unique_ptr<sdbus::IObject> dbus_object;
    void list(sdbus::MethodCall call);
    void get(sdbus::MethodCall call);
    void enable_disable(sdbus::MethodCall call, const bool & enable);
    bool check_authorization(const std::string & actionid, const std::string & sender);

    dnfdaemon::KeyValueMapList repo_list(const std::vector<std::string> & ids);
    std::vector<std::string> enable_disable_repos(const std::vector<std::string> & ids, const bool enable);
};

#endif
