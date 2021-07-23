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

#ifndef DNFDAEMON_CLIENT_CONTEXT_HPP
#define DNFDAEMON_CLIENT_CONTEXT_HPP

#include "callbacks.hpp"
#include "commands/command.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <libdnf-cli/argument_parser.hpp>
#include <libdnf-cli/session.hpp>
#include <libdnf/base/base.hpp>
#include <libdnf/conf/config.hpp>
#include <libdnf/repo/repo.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <memory>
#include <utility>
#include <vector>


namespace dnfdaemon::client {

constexpr const char * VERSION = "0.1.0";

class Context : public libdnf::cli::session::Session {
public:
    Context() : repositories_status(dnfdaemon::RepoStatus::NOT_READY){};

    /// Initialize dbus connection and server session
    void init_session(sdbus::IConnection & connection);
    sdbus::ObjectPath & get_session_object_path() { return session_object_path; };

    // initialize repository metadata loading on server side and wait for results
    dnfdaemon::RepoStatus wait_for_repos();

    // signal handlers
    void on_repositories_ready(const bool & result);

    /// proxy to dnfdaemon session
    std::unique_ptr<sdbus::IProxy> session_proxy;

    // global command line arguments
    std::vector<std::pair<std::string, std::string>> setopts;
    libdnf::OptionBool verbose{false};
    libdnf::OptionBool assume_yes{false};
    libdnf::OptionBool assume_no{false};
    libdnf::OptionBool allow_erasing{false};
    libdnf::OptionString installroot{"/"};
    libdnf::OptionString releasever{""};

private:
    sdbus::ObjectPath session_object_path;
    dnfdaemon::RepoStatus repositories_status;
    std::unique_ptr<RepoCB> repocb;
    std::unique_ptr<PackageDownloadCB> package_download_cb;
    std::unique_ptr<TransactionCB> transaction_cb;
};

}  // namespace dnfdaemon::client

#endif
