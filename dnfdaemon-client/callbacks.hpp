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
#include <libdnf-cli/progressbar/multi_progress_bar.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <string>

namespace dnfdaemon::client {

class DbusCallback {
public:
    explicit DbusCallback(sdbus::IProxy * proxy, std::string session_object_path)
        : session_object_path(session_object_path)
        , proxy(proxy){};
    virtual ~DbusCallback() = default;

protected:
    std::string session_object_path;
    sdbus::IProxy * proxy;

    bool signature_valid(sdbus::Signal & signal);
};


class RepoCB final : public DbusCallback {
public:
    explicit RepoCB(sdbus::IProxy * proxy, std::string session_object_path);
    virtual ~RepoCB() = default;

    void start(sdbus::Signal & signal);
    void end(sdbus::Signal & signal);
    void progress(sdbus::Signal & signal);

private:
    libdnf::cli::progressbar::DownloadProgressBar progress_bar{-1, ""};
    std::size_t msg_lines{0};
    void print_progress_bar();
};


class PackageDownloadCB final : public DbusCallback {
public:
    explicit PackageDownloadCB(sdbus::IProxy * proxy, std::string session_object_path);
    virtual ~PackageDownloadCB() = default;

    void start(sdbus::Signal & signal);
    void end(sdbus::Signal & signal);
    void progress(sdbus::Signal & signal);
    void mirror_failure(sdbus::Signal & signal);

private:
    libdnf::cli::progressbar::MultiProgressBar multi_progress_bar;
    // map {package nevra: progressbar}
    std::map<std::string, std::unique_ptr<libdnf::cli::progressbar::DownloadProgressBar>> package_bars;

    libdnf::cli::progressbar::DownloadProgressBar * find_progress_bar(std::string nevra) {
        if (package_bars.find(nevra) != package_bars.end()) {
            return package_bars.at(nevra).get();
        } else {
            return nullptr;
        }
    }
};

}  // namespace dnfdaemon::client

#endif
