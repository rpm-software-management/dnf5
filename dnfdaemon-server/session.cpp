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
#include "services/base/base.hpp"
#include "services/repo/repo.hpp"
#include "services/repoconf/repo_conf.hpp"

#include <libdnf/logger/logger.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
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

Session::Session(sdbus::IConnection & connection, dnfdaemon::KeyValueMap session_configuration, std::string object_path)
    : connection(connection)
    , base(std::make_unique<libdnf::Base>())
    , session_configuration(session_configuration)
    , object_path(object_path) {


    // adjust base.config from session_configuration
    auto & config = base->get_config();
    std::vector<std::string> config_items {"config_file_path", "installroot", "cachedir", "reposdir", "varsdir"};
    for (auto & key: config_items) {
        if (session_configuration.find(key) != session_configuration.end()) {
            auto value = session_configuration_value<std::string>(key);
            config.opt_binds().at(key).new_string(libdnf::Option::Priority::RUNTIME, value);
        }
    }

    // set-up log router for base
    auto & log_router = base->get_logger();
    log_router.add_logger(std::make_unique<StderrLogger>());

    // load configuration
    base->load_config_from_file();

    // set cachedir
    auto system_cache_dir = config.system_cachedir().get_value();
    config.cachedir().set(libdnf::Option::Priority::RUNTIME, system_cache_dir);
    // set variables
    base->get_vars().load(
        config.installroot().get_value(),
        config.varsdir().get_value()
    );

    // load repo configuration
    auto & rpm_repo_sack = base->get_rpm_repo_sack();
    rpm_repo_sack.new_repos_from_file();
    rpm_repo_sack.new_repos_from_dirs();

    // instantiate all services provided by the daemon
    services.emplace_back(std::make_unique<Base>(*this));
    services.emplace_back(std::make_unique<RepoConf>(*this));
    services.emplace_back(std::make_unique<Repo>(*this));

    // Register all provided services on d-bus
    for (auto & s : services) {
        s->dbus_register();
    }
}

Session::~Session() {
    // deregister dbus services
    for (auto & s : services) {
        s->dbus_deregister();
    }
    threads_manager.finish();
}
