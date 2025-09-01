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

#ifndef DNF5DAEMON_CLIENT_CONTEXT_HPP
#define DNF5DAEMON_CLIENT_CONTEXT_HPP

#include "callbacks.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/conf/config.hpp>
#include <libdnf5/repo/repo.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <memory>
#include <utility>
#include <vector>


namespace dnfdaemon::client {

constexpr const char * VERSION = "0.1.0";

class Context : public libdnf5::cli::session::Session {
public:
    Context() : repositories_status(dnfdaemon::RepoStatus::NOT_READY) {};

    /// Initialize dbus connection and server session
    void init_session(sdbus::IConnection & connection);
    sdbus::ObjectPath get_session_object_path() { return session_object_path; };

    // signal handlers
    void on_repositories_ready(const bool & result);

    /// proxy to dnf5daemon session
    std::unique_ptr<sdbus::IProxy> session_proxy;

    libdnf5::OptionBool get_assumeno_option() const { return assume_no; }
    libdnf5::OptionBool get_assumeyes_option() const { return assume_yes; }
    libdnf5::OptionBool get_defaultyes_option() const { return default_yes; }

    // global command line arguments
    std::vector<std::pair<std::string, std::string>> setopts;
    libdnf5::OptionBool verbose{false};
    libdnf5::OptionBool assume_yes{false};
    libdnf5::OptionBool assume_no{false};
    libdnf5::OptionBool default_yes{false};
    libdnf5::OptionBool allow_erasing{false};
    libdnf5::OptionString installroot{"/"};
    libdnf5::OptionString releasever{""};

    void reset_download_cb();
    void set_download_cb(DownloadCB * download_cb) { this->download_cb = download_cb; }

private:
    sdbus::ObjectPath session_object_path;
    dnfdaemon::RepoStatus repositories_status;
    DownloadCB * download_cb{nullptr};
};

}  // namespace dnfdaemon::client

#endif
