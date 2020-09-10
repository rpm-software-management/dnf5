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

#include "session.hpp"

#include "dbus.hpp"
#include "services/repo/repo.hpp"
#include "services/repoconf/repo_conf.hpp"
#include "utils.hpp"

#include <libdnf/logger/logger.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <string>

class StderrLogger : public libdnf::Logger {
public:
    explicit StderrLogger() {}
    void write(time_t, pid_t, Level, const std::string & message) noexcept override;
};

void StderrLogger::write(time_t, pid_t, Level, const std::string & message) noexcept {
    try {
        std::cerr << message << std::endl;
    } catch (...) {
    }
}

template <typename ItemType>
ItemType Session::session_configuration_value(const std::string & key, const ItemType & default_value) {
    return key_value_map_get(session_configuration, key, default_value);
}


Session::Session(sdbus::IConnection & connection, dnfdaemon::KeyValueMap session_configuration, std::string object_path)
    : connection(connection)
    , base(std::make_unique<libdnf::Base>())
    , session_configuration(session_configuration)
    , object_path(object_path) {

    // set-up log router for base
    auto & log_router = base->get_logger();
    log_router.add_logger(std::make_unique<StderrLogger>());

    // load configuration
    base->load_config_from_file();

    // set cachedir
    auto system_cache_dir = base->get_config().system_cachedir().get_value();
    base->get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, system_cache_dir);
    // set variables
    auto & variables = base->get_variables();
    variables["arch"] = "x86_64";
    variables["basearch"] = "x86_64";
    variables["releasever"] = "32";

    // load repo configuration
    auto & rpm_repo_sack = base->get_rpm_repo_sack();
    rpm_repo_sack.new_repos_from_file();
    rpm_repo_sack.new_repos_from_dirs();

    // instantiate all services provided by the daemon
    services.emplace_back(std::make_unique<RepoConf>(*this));
    services.emplace_back(std::make_unique<Repo>(*this));

    // Register all provided services on d-bus
    for (auto & s : services) {
        s->dbus_register();
    }
}

Session::~Session() {
    for (auto & s : services) {
        s->dbus_deregister();
    }
}

// explicit instantiation of session_configuration_value template
template std::string Session::session_configuration_value<std::string>(const std::string &, const std::string &);
template int Session::session_configuration_value<int>(const std::string &, const int &);
