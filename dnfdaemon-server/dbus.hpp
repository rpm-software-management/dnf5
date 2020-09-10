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



// constants

const char * const DBUS_NAME = "org.rpm.dnf.v0";
const char * const DBUS_OBJECT_PATH = "/org/rpm/dnf/v0";

// interfaces
const char * const INTERFACE_BASE = "org.rpm.dnf.v0.Base";
const char * const INTERFACE_REPO = "org.rpm.dnf.v0.rpm.Repo";
const char * const INTERFACE_REPOCONF = "org.rpm.dnf.v0.rpm.RepoConf";
const char * const INTERFACE_SESSION_MANAGER = "org.rpm.dnf.v0.SessionManager";

// polkit actions
const char * const POLKIT_REPOCONF_WRITE = "org.rpm.dnf.v0.rpm.RepoConf.write";

// errors
const char * const ERROR = "org.rpm.dnf.v0.Error";
const char * const ERROR_REPOCONF = "org.rpm.dnf.v0.rpm.RepoConf.Error";

}  // namespace dnfdaemon

#endif
