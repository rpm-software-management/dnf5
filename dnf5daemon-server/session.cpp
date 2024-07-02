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
#include "services/advisory/advisory.hpp"
#include "services/base/base.hpp"
#include "services/comps/group.hpp"
#include "services/goal/goal.hpp"
#include "services/offline/offline.hpp"
#include "services/repo/repo.hpp"
#include "services/rpm/rpm.hpp"
#include "utils.hpp"

#include <libdnf5/conf/const.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/transaction/offline.hpp>
#include <libdnf5/utils/fs/file.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

// config options that regular user can override for their session.
static const std::unordered_set<std::string> ALLOWED_MAIN_CONF_OVERRIDES = {
    "allow_downgrade",
    "allow_vendor_change",
    "best",
    "clean_requirements_on_remove",
    "disable_excludes",
    "exclude_from_weak",
    "exclude_from_weak_autodetect",
    "excludepkgs",
    "ignorearch",
    "includepkgs",
    "installonly_limit",
    "installonlypkgs",
    "install_weak_deps",
    "keepcache",
    "module_obsoletes",
    "module_platform_id",
    "module_stream_switch",
    "multilib_policy",
    "obsoletes",
    "optional_metadata_types",
    "protect_running_kernel",
    "skip_broken",
    "skip_if_unavailable",
    "skip_unavailable",
    "strict",
};

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
    std::optional<bool> am_i_root;
    for (auto & opt : conf_overrides) {
        auto key = opt.first;
        auto value = opt.second;
        auto bind = opt_binds.find(key);
        if (bind != opt_binds.end()) {
            if (ALLOWED_MAIN_CONF_OVERRIDES.find(key) != ALLOWED_MAIN_CONF_OVERRIDES.end()) {
                bind->second.new_string(libdnf5::Option::Priority::RUNTIME, value);
            } else {
                if (!am_i_root.has_value()) {
                    // check the authorization lazily only once really needed
                    am_i_root = check_authorization(dnfdaemon::POLKIT_CONFIG_OVERRIDE, sender, false);
                }
                // restricted config options override is allowed only for root
                if (am_i_root.value()) {
                    bind->second.new_string(libdnf5::Option::Priority::RUNTIME, value);
                } else {
                    base->get_logger()->warning("Config option {} not allowed.", key);
                }
            }
        } else {
            base->get_logger()->warning("Unknown config option: {}", key);
        }
    }

    // load configuration
    base->load_config();

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
    services.emplace_back(std::make_unique<Repo>(*this));
    services.emplace_back(std::make_unique<Rpm>(*this));
    services.emplace_back(std::make_unique<Goal>(*this));
    services.emplace_back(std::make_unique<Offline>(*this));
    services.emplace_back(std::make_unique<Group>(*this));
    services.emplace_back(std::make_unique<dnfdaemon::Advisory>(*this));

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
    std::vector<std::string> optional_metadata_str =
        session_configuration_value<std::vector<std::string>>("optional_metadata_types", {});
    if (load_available_repos) {
        auto optional_metadata_types_opt = base->get_config().get_optional_metadata_types_option();
        for (const auto & type : optional_metadata_str) {
            optional_metadata_types_opt.add_item(libdnf5::Option::Priority::RUNTIME, type);
        }
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
            if (load_system_repo) {
                base->get_repo_sack()->load_repos();
            } else {
                base->get_repo_sack()->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);
            }
        } catch (const std::runtime_error & ex) {
            retval = false;
        }
    } else if (load_system_repo) {
        base->get_repo_sack()->load_repos(libdnf5::repo::Repo::Type::SYSTEM);
    }

    repositories_status = retval ? dnfdaemon::RepoStatus::READY : dnfdaemon::RepoStatus::ERROR;
    return retval;
}

bool Session::check_authorization(
    const std::string & actionid, const std::string & sender, bool allow_user_interaction) {
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
    // allow polkit to ask user to enter root password
    uint flags = allow_user_interaction ? 1 : 0;
    std::string cancellation_id = "";
    try {
        polkit_proxy->callMethod("CheckAuthorization")
            .onInterface(interface_name)
            .withArguments(subject, actionid, details, flags, cancellation_id)
            .withTimeout(std::chrono::minutes(2))
            .storeResultsTo(auth_result);
    } catch (const sdbus::Error & ex) {
        auto name = ex.getName();
        if (name == "org.freedesktop.DBus.Error.Timeout" || name == "org.freedesktop.DBus.Error.NoReply") {
            // in case of timeout return "not authorized"
            return false;
        }
        throw sdbus::Error(dnfdaemon::ERROR, fmt::format("Failed to check authorization: \"{}\"", ex.what()));
    }

    // get results
    bool res_is_authorized = std::get<0>(auth_result);
    /*
    bool res_is_challenge = std::get<1>(auth_result);
    std::map<std::string, std::string> res_details = std::get<2>(auth_result);
    */

    return res_is_authorized;
}

void Session::download_transaction_packages() {
    libdnf5::repo::PackageDownloader downloader(base->get_weak_ptr());

    // container is owner of package callbacks user_data
    std::vector<std::unique_ptr<dnf5daemon::DownloadUserData>> user_data;
    for (auto & tspkg : transaction->get_transaction_packages()) {
        if (transaction_item_action_is_inbound(tspkg.get_action()) &&
            tspkg.get_package().get_repo()->get_type() != libdnf5::repo::Repo::Type::COMMANDLINE) {
            auto & data = user_data.emplace_back(std::make_unique<dnf5daemon::DownloadUserData>());
            data->download_id = "package:" + std::to_string(tspkg.get_package().get_id().id);
            downloader.add(tspkg.get_package(), data.get());
        }
    }

    downloader.download();
}

void Session::store_transaction_offline() {
    const auto & installroot = base->get_config().get_installroot_option().get_value();
    const auto & dest_dir = installroot / libdnf5::offline::DEFAULT_DATADIR.relative_path() / "packages";
    std::filesystem::create_directories(dest_dir);
    base->get_config().get_destdir_option().set(dest_dir);
    download_transaction_packages();

    const auto & offline_datadir = installroot / libdnf5::offline::DEFAULT_DATADIR.relative_path();
    std::filesystem::create_directories(offline_datadir);

    constexpr const char * packages_in_trans_dir{"./packages"};
    constexpr const char * comps_in_trans_dir{"./comps"};
    const auto & comps_location = offline_datadir / comps_in_trans_dir;

    const std::filesystem::path state_path{offline_datadir / libdnf5::offline::TRANSACTION_STATE_FILENAME};
    libdnf5::offline::OfflineTransactionState state{state_path};

    auto & state_data = state.get_data();

    state_data.set_status(libdnf5::offline::STATUS_DOWNLOAD_INCOMPLETE);
    state.write();

    // First, serialize the transaction
    transaction->store_comps(comps_location);

    const auto transaction_json_path = offline_datadir / "transaction.json";
    libdnf5::utils::fs::File transaction_json_file{transaction_json_path, "w"};
    transaction_json_file.write(transaction->serialize(packages_in_trans_dir, comps_in_trans_dir));
    transaction_json_file.close();

    // Then, test the serialized transaction
    // TODO(mblaha): store transaction test/run problems in the session and add an API
    // to retrieve it
    const auto & test_goal = std::make_unique<libdnf5::Goal>(*base);
    test_goal->add_serialized_transaction(transaction_json_path);
    auto test_transaction = test_goal->resolve();
    if (test_transaction.get_problems() != libdnf5::GoalProblem::NO_PROBLEM) {
        throw sdbus::Error(dnfdaemon::ERROR_TRANSACTION, "failed to resolve serialized offline transaction.");
    }
    base->get_config().get_tsflags_option().set(libdnf5::Option::Priority::RUNTIME, "test");

    auto result = test_transaction.run();
    if (result != libdnf5::base::Transaction::TransactionRunResult::SUCCESS) {
        throw sdbus::Error(
            dnfdaemon::ERROR_TRANSACTION,
            fmt::format(
                "offline rpm transaction test failed with code {}.",
                static_cast<std::underlying_type_t<libdnf5::base::Transaction::TransactionRunResult>>(result)));
    }

    // Download and transaction test complete. Fill out entries in offline
    // transaction state file.
    state_data.set_cachedir(base->get_config().get_cachedir_option().get_value());

    state_data.set_cmd_line("dnf5daemon-server");

    const auto & detected_releasever = libdnf5::Vars::detect_release(base->get_weak_ptr(), installroot);
    if (detected_releasever != nullptr) {
        state_data.set_system_releasever(*detected_releasever);
    }
    state_data.set_target_releasever(base->get_vars()->get_value("releasever"));

    const auto module_platform_id = base->get_config().get_module_platform_id_option();
    if (!module_platform_id.empty()) {
        state_data.set_module_platform_id(module_platform_id.get_value());
    }

    // create the magic symlink /system-update -> datadir
    if (!std::filesystem::is_symlink(libdnf5::offline::MAGIC_SYMLINK)) {
        std::filesystem::create_symlink(offline_datadir, libdnf5::offline::MAGIC_SYMLINK);
    }
    state_data.set_status(libdnf5::offline::STATUS_READY);

    state.write();
}
