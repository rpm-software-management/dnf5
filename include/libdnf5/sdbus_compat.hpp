// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

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
