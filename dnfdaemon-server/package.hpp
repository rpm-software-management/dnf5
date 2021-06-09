/*
Copyright (C) 2021 Red Hat, Inc.

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

#ifndef DNFDAEMON_SERVER_PACKAGE_HPP
#define DNFDAEMON_SERVER_PACKAGE_HPP

#include "dbus.hpp"

#include <libdnf/rpm/package.hpp>

#include <string>
#include <vector>
#include <map>

// TODO(mblaha): add all other package attributes
// package attributes available to be retrieved
enum class PackageAttribute {
    name,
    epoch,
    version,
    release,
    arch,
    repo,
    is_installed,
    install_size,
    package_size,

    evr,
    nevra,
    full_nevra
};

dnfdaemon::KeyValueMap package_to_map(
    const libdnf::rpm::Package & libdnf_package, const std::vector<std::string> & attributes);

#endif
