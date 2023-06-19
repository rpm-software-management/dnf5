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

#include "session.hpp"

#include "callbacks.hpp"
#include "dbus.hpp"
#include "services/base/base.hpp"
#include "services/comps/group.hpp"
#include "services/goal/goal.hpp"
#include "services/repo/repo.hpp"
#include "services/repoconf/repo_conf.hpp"
#include "services/rpm/rpm.hpp"
#include "utils.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <iostream>
#include <string>


Session::Session(
    std::vector<std::unique_ptr<libdnf5::Logger>> && loggers,
    sdbus::IConnection & connection,
    dnfdaemon::KeyValueMap session_configuration,
    std::string object_path,
    std::string sender)
    : connection(connection),
      base(std::make_unique<libdnf5::Base>(std::move(loggers))),
      goal(*base),
      session_configuration(session_configuration),
      object_path(object_path),
      sender(sender) {
    if (session_configuration.find("locale") != session_configuration.end()) {
        session_locale = session_configuration_value<std::string>("locale");
    }

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
            bind->second.new_string(libdnf5::Option::Priority::RUNTIME, value);
        } else {
            base->get_logger()->warning("Unknown config option: {}", key);
        }
    }

    // load configuration
    base->load_config_from_file();

    // set variables
    base->setup();
    if (session_configuration.find("releasever") != session_configuration.end()) {
        auto releasever = session_configuration_value<std::string>("releasever");
        base->get_vars()->set("releasever", releasever);
    }

    // load repo configuration
    base->get_repo_sack()->create_repos_from_system_configuration();

    // instantiate all services provided by the daemon
    services.emplace_back(std::make_unique<Base>(*this));
    services.emplace_back(std::make_unique<RepoConf>(*this));
    services.emplace_back(std::make_unique<Repo>(*this));
    services.emplace_back(std::make_unique<Rpm>(*this));
    services.emplace_back(std::make_unique<Goal>(*this));
    services.emplace_back(std::make_unique<Group>(*this));

    dbus_object = sdbus::createObject(connection, object_path);
    // Register all provided services on d-bus
    for (auto & s : services) {
        s->dbus_register();
    }
    dbus_object->finishRegistration();

    base->set_download_callbacks(std::make_unique<dnf5daemon::DownloadCB>(*this));
}

Session::~Session() {
    dbus_object->unregister();
    threads_manager.finish();
}

void Session::confirm_key(const std::string & key_id, const bool confirmed) {
    std::lock_guard<std::mutex> lock(key_import_mutex);
    auto status_it = key_import_status.find(key_id);
    // ignore replies which were not requested
    if (status_it != key_import_status.end()) {
        // ignore confirmations that has been already answered
        if (status_it->second == KeyConfirmationStatus::PENDING) {
            key_import_status[key_id] = confirmed ? KeyConfirmationStatus::CONFIRMED : KeyConfirmationStatus::REJECTED;
            key_import_condition.notify_all();
        }
    }
}

bool Session::wait_for_key_confirmation(const std::string & key_id, sdbus::Signal & request_signal) {
    std::unique_lock<std::mutex> key_import_lock(key_import_mutex);
    if (key_import_status.find(key_id) == key_import_status.end()) {
        // signal client that repository key import confirmation is required
        key_import_status[key_id] = KeyConfirmationStatus::PENDING;
        dbus_object->emitSignal(request_signal);
    }

    // wait for a confirmation for <timeout>
    auto timeout = std::chrono::minutes(5);
    auto wait = key_import_condition.wait_for(key_import_lock, timeout, [this, &key_id]() {
        return key_import_status.at(key_id) != KeyConfirmationStatus::PENDING;
    });
    if (!wait) {
        key_import_lock.unlock();
        throw sdbus::Error(dnfdaemon::ERROR, "Timeout while waiting for the repository key import confirmation.");
    }
    auto confirmation = key_import_status.at(key_id);
    key_import_lock.unlock();

    return confirmation == KeyConfirmationStatus::CONFIRMED;
}

void Session::fill_sack() {
    if (!read_all_repos()) {
        throw std::runtime_error("Cannot load repositories.");
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

    bool retval = true;

    bool load_available_repos = session_configuration_value<bool>("load_available_repos", true);
    bool load_system_repo = session_configuration_value<bool>("load_system_repo", true);
    if (load_available_repos) {
        //auto & logger = base->get_logger();
        libdnf5::repo::RepoQuery enabled_repos(*base);
        enabled_repos.filter_enabled(true);
        enabled_repos.filter_type(libdnf5::repo::Repo::Type::AVAILABLE);
        // container is owner of package callbacks user_data
        std::vector<std::unique_ptr<dnf5daemon::DownloadUserData>> repos_user_data;
        for (auto & repo : enabled_repos) {
            auto & user_data = repos_user_data.emplace_back(std::make_unique<dnf5daemon::DownloadUserData>());
            user_data->download_id = "repo:" + repo->get_id();
            repo->set_user_data(user_data.get());
            repo->set_callbacks(std::make_unique<dnf5daemon::KeyImportRepoCB>(*this));
        }

        try {
            base->get_repo_sack()->update_and_load_enabled_repos(load_system_repo);
        } catch (const std::runtime_error & ex) {
            retval = false;
        }
    } else if (load_system_repo) {
        base->get_repo_sack()->get_system_repo()->load();
    }

    repositories_status = retval ? dnfdaemon::RepoStatus::READY : dnfdaemon::RepoStatus::ERROR;
    return retval;
}

bool Session::check_authorization(const std::string & actionid, const std::string & sender) {
    // create proxy for PolicyKit1 object
    const std::string destination_name = "org.freedesktop.PolicyKit1";
    const std::string object_path = "/org/freedesktop/PolicyKit1/Authority";
    const std::string interface_name = "org.freedesktop.PolicyKit1.Authority";
    // allow polkit to ask user to enter root password
    const uint ALLOW_USER_INTERACTION = 1;
    auto polkit_proxy = sdbus::createProxy(connection, destination_name, object_path);
    polkit_proxy->finishRegistration();

    // call CheckAuthorization method
    sdbus::Struct<bool, bool, std::map<std::string, std::string>> auth_result;
    sdbus::Struct<std::string, dnfdaemon::KeyValueMap> subject{"system-bus-name", {{"name", sender}}};
    std::map<std::string, std::string> details{};
    uint flags = ALLOW_USER_INTERACTION;
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
