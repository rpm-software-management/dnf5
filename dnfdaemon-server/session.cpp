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

#include "callbacks.hpp"
#include "dbus.hpp"
#include "services/base/base.hpp"
#include "services/goal/goal.hpp"
#include "services/repo/repo.hpp"
#include "services/repoconf/repo_conf.hpp"
#include "services/rpm/rpm.hpp"
#include "utils.hpp"

#include <libdnf/logger/logger.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <iostream>
#include <string>

namespace {

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

}  // namespace

Session::Session(
    sdbus::IConnection & connection,
    dnfdaemon::KeyValueMap session_configuration,
    std::string object_path,
    std::string sender)
    : connection(connection),
      base(std::make_unique<libdnf::Base>()),
      goal(*base),
      session_configuration(session_configuration),
      object_path(object_path),
      sender(sender) {
    // set-up log router for base
    auto & logger = *base->get_logger();
    logger.add_logger(std::make_unique<StderrLogger>());

    auto & config = base->get_config();

    // adjust base.config from session_configuration config overrides
    std::map<std::string, std::string> default_overrides{};
    auto conf_overrides = session_configuration_value<std::map<std::string, std::string>>("config", default_overrides);
    auto opt_binds = config.opt_binds();
    for (auto & opt : conf_overrides) {
        auto key = opt.first;
        auto value = opt.second;
        auto bind = opt_binds.find(key);
        if (bind != opt_binds.end()) {
            bind->second.new_string(libdnf::Option::Priority::RUNTIME, value);
        } else {
            logger.warning(fmt::format("Unknown config option: {}", key));
        }
    }

    // load configuration
    base->load_config_from_file();

    // set cachedir
    auto system_cache_dir = config.system_cachedir().get_value();
    config.cachedir().set(libdnf::Option::Priority::DEFAULT, system_cache_dir);
    // set variables
    base->get_vars()->load(config.installroot().get_value(), config.varsdir().get_value());
    if (session_configuration.find("releasever") != session_configuration.end()) {
        auto releasever = session_configuration_value<std::string>("releasever");
        base->get_vars()->set("releasever", releasever);
    }

    // load repo configuration
    auto repo_sack = base->get_repo_sack();
    repo_sack->new_repos_from_file();
    repo_sack->new_repos_from_dirs();

    // instantiate all services provided by the daemon
    services.emplace_back(std::make_unique<Base>(*this));
    services.emplace_back(std::make_unique<RepoConf>(*this));
    services.emplace_back(std::make_unique<Repo>(*this));
    services.emplace_back(std::make_unique<Rpm>(*this));
    services.emplace_back(std::make_unique<Goal>(*this));

    dbus_object = sdbus::createObject(connection, object_path);
    // Register all provided services on d-bus
    for (auto & s : services) {
        s->dbus_register();
    }
    dbus_object->finishRegistration();
}

Session::~Session() {
    dbus_object->unregister();
    threads_manager.finish();
}

void Session::fill_sack() {
    auto & package_sack = *get_base()->get_rpm_package_sack();
    if (session_configuration_value<bool>("load_system_repo", true)) {
        package_sack.create_system_repo(false);
    }

    if (session_configuration_value<bool>("load_available_repos", true)) {
        if (!read_all_repos()) {
            throw std::runtime_error("Cannot load repositories.");
        }
    }
}

bool Session::read_all_repos() {
    while (repositories_status == dnfdaemon::RepoStatus::PENDING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (repositories_status == dnfdaemon::RepoStatus::READY) {
        return true;
    } else if (repositories_status == dnfdaemon::RepoStatus::ERROR) {
        return false;
    }
    repositories_status = dnfdaemon::RepoStatus::PENDING;
    // TODO(mblaha): get flags from session configuration
    using LoadFlags = libdnf::rpm::PackageSack::LoadRepoFlags;
    auto flags = LoadFlags::USE_FILELISTS | LoadFlags::USE_PRESTO | LoadFlags::USE_UPDATEINFO | LoadFlags::USE_OTHER;
    //auto & logger = base->get_logger();
    libdnf::repo::RepoQuery enabled_repos(*base);
    enabled_repos.filter_enabled(true);
    auto & package_sack = *base->get_rpm_package_sack();
    bool retval = true;
    for (auto & repo : enabled_repos.get_data()) {
        repo->set_callbacks(std::make_unique<DbusRepoCB>(*this));
        try {
            repo->load();
            package_sack.load_repo(*repo.get(), flags);
        } catch (const std::runtime_error & ex) {
            if (!repo->get_config().skip_if_unavailable().get_value()) {
                retval = false;
                break;
            }
        }
        // reset callbacks - otherwise progress callback is called later on during
        // packages download when internal mirrorlist in librepo is prepared.
        // (lr_download_packages() -> lr_handle_prepare_internal_mirrorlist())
        repo->set_callbacks(nullptr);
    }
    repositories_status = retval ? dnfdaemon::RepoStatus::READY : dnfdaemon::RepoStatus::ERROR;
    return retval;
}

bool Session::check_authorization(const std::string & actionid, const std::string & sender) {
    // create proxy for PolicyKit1 object
    const std::string destination_name = "org.freedesktop.PolicyKit1";
    const std::string object_path = "/org/freedesktop/PolicyKit1/Authority";
    const std::string interface_name = "org.freedesktop.PolicyKit1.Authority";
    auto polkit_proxy = sdbus::createProxy(connection, destination_name, object_path);
    polkit_proxy->finishRegistration();

    // call CheckAuthorization method
    sdbus::Struct<bool, bool, std::map<std::string, std::string>> auth_result;
    sdbus::Struct<std::string, dnfdaemon::KeyValueMap> subject{"system-bus-name", {{"name", sender}}};
    std::map<std::string, std::string> details{};
    uint flags = 0;
    std::string cancelation_id = "";
    polkit_proxy->callMethod("CheckAuthorization")
        .onInterface(interface_name)
        .withArguments(subject, actionid, details, flags, cancelation_id)
        .storeResultsTo(auth_result);

    // get results
    bool res_is_authorized = std::get<0>(auth_result);
    /*
    bool res_is_challenge = std::get<1>(auth_result);
    std::map<std::string, std::string> res_details = std::get<2>(auth_result);
    */

    return res_is_authorized;
}
