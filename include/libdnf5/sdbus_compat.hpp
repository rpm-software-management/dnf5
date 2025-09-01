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

#ifndef DNF5DAEMON_SERVER_SDBUS_COMPAT_HPP
#define DNF5DAEMON_SERVER_SDBUS_COMPAT_HPP

#ifdef SDBUS_CPP_VERSION_2

#define SDBUS_INTERFACE_NAME_TYPE sdbus::InterfaceName
#define SDBUS_SIGNAL_NAME_TYPE    sdbus::SignalName
#define SDBUS_SERVICE_NAME_TYPE   sdbus::ServiceName
#define SDBUS_ERROR_NAME_TYPE     sdbus::Error::Name

#else

#define SDBUS_INTERFACE_NAME_TYPE std::string
#define SDBUS_SIGNAL_NAME_TYPE    std::string
#define SDBUS_SERVICE_NAME_TYPE   std::string
#define SDBUS_ERROR_NAME_TYPE     std::string

#endif

#endif
