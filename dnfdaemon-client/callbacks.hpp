/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNFDAEMON_CLIENT_CALLBACKS_HPP
#define DNFDAEMON_CLIENT_CALLBACKS_HPP

#include <libdnf-cli/progressbar/download_progress_bar.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <string>

namespace dnfdaemon::client {

class RepoCB {
public:
    explicit RepoCB(sdbus::IProxy * proxy, std::string session_object_path);

    void start(sdbus::Signal & signal);
    void end(sdbus::Signal & signal);
    void progress(sdbus::Signal & signal);

private:
    std::string session_object_path;
    libdnf::cli::progressbar::DownloadProgressBar progress_bar{-1, ""};
    std::size_t msg_lines{0};
    void print_progress_bar();
    bool signature_valid(sdbus::Signal & signal);
};

}  // namespace dnfdaemon::client

#endif
