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

#include "base.hpp"

#include "dbus.hpp"
#include "utils.hpp"
#include "utils/string.hpp"

#include <fmt/format.h>
#include <libdnf5/repo/repo.hpp>
#include <libdnf5/repo/repo_cache.hpp>
#include <sdbus-c++/sdbus-c++.h>
#include <unistd.h>

#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>

static const std::unordered_set<std::string> ALLOWED_CACHE_TYPES = {
    "all",
    "packages",
    "metadata",
    "dbcache",
    "expire-cache",
};

void Base::dbus_register() {
    auto dbus_object = session.get_dbus_object();
#ifdef SDBUS_CPP_VERSION_2
    dbus_object
        ->addVTable(
            sdbus::MethodVTableItem{
                sdbus::MethodName{"read_all_repos"},
                sdbus::Signature{""},
                {},
                sdbus::Signature{"b"},
                {"success"},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(
                        *this, &Base::read_all_repos, call, session.session_locale);
                },
                {}},
            sdbus::MethodVTableItem{
                sdbus::MethodName{"clean_with_options"},
                sdbus::Signature{"sa{sv}"},
                {"cache_type", "options"},
                sdbus::Signature{"bs"},
                {"success", "error_msg"},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(
                        *this, &Base::clean_with_options, call, session.session_locale);
                },
                {}},
            sdbus::MethodVTableItem{
                sdbus::MethodName{"clean"},
                sdbus::Signature{"s"},
                {"cache_type"},
                sdbus::Signature{"bs"},
                {"success", "error_msg"},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(*this, &Base::clean, call, session.session_locale);
                },
                {}},
            sdbus::MethodVTableItem{
                sdbus::MethodName{"reset"},
                sdbus::Signature{""},
                {},
                sdbus::Signature{"bs"},
                {"success", "error_msg"},
                [this](sdbus::MethodCall call) -> void {
                    session.get_threads_manager().handle_method(*this, &Base::reset, call, session.session_locale);
                },
                {}},
            sdbus::SignalVTableItem{
                dnfdaemon::SIGNAL_DOWNLOAD_ADD_NEW,
                sdbus::Signature{"ossx"},
                {"session_object_path", "download_id", "description", "total_to_download"},
                {}},
            sdbus::SignalVTableItem{
                dnfdaemon::SIGNAL_DOWNLOAD_PROGRESS,
                sdbus::Signature{"osxx"},
                {"session_object_path", "download_id", "total_to_download", "downloaded"},
                {}},
            sdbus::SignalVTableItem{
                dnfdaemon::SIGNAL_DOWNLOAD_END,
                sdbus::Signature{"osus"},
                {"session_object_path", "download_id", "transfer_status", "message"},
                {}},
            sdbus::SignalVTableItem{
                dnfdaemon::SIGNAL_DOWNLOAD_MIRROR_FAILURE,
                sdbus::Signature{"ossss"},
                {"session_object_path", "download_id", "message", "url", "metadata"},
                {}},
            sdbus::SignalVTableItem{
                dnfdaemon::SIGNAL_REPO_KEY_IMPORT_REQUEST,
                sdbus::Signature{"osasssx"},
                {"session_object_path", "key_id", "user_ids", "key_fingerprint", "key_url", "timestamp"},
                {}})
        .forInterface(dnfdaemon::INTERFACE_BASE);
#else
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_BASE, "read_all_repos", "", {}, "b", {"success"}, [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Base::read_all_repos, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_BASE,
        "clean_with_options",
        "sa{sv}",
        {"cache_type", "options"},
        "bs",
        {"success", "error_msg"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Base::clean_with_options, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_BASE,
        "clean",
        "s",
        {"cache_type"},
        "bs",
        {"success", "error_msg"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Base::clean, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_BASE,
        "reset",
        "",
        {},
        "bs",
        {"success", "error_msg"},
        [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Base::reset, call, session.session_locale);
        });

    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_BASE,
        dnfdaemon::SIGNAL_DOWNLOAD_ADD_NEW,
        "ossx",
        {"session_object_path", "download_id", "description", "total_to_download"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_BASE,
        dnfdaemon::SIGNAL_DOWNLOAD_PROGRESS,
        "osxx",
        {"session_object_path", "download_id", "total_to_download", "downloaded"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_BASE,
        dnfdaemon::SIGNAL_DOWNLOAD_END,
        "osus",
        {"session_object_path", "download_id", "transfer_status", "message"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_BASE,
        dnfdaemon::SIGNAL_DOWNLOAD_MIRROR_FAILURE,
        "ossss",
        {"session_object_path", "download_id", "message", "url", "metadata"});
    dbus_object->registerSignal(
        dnfdaemon::INTERFACE_BASE,
        dnfdaemon::SIGNAL_REPO_KEY_IMPORT_REQUEST,
        "osasssx",
        {"session_object_path", "key_id", "user_ids", "key_fingerprint", "key_url", "timestamp"});
#endif
}

sdbus::MethodReply Base::read_all_repos(sdbus::MethodCall & call) {
    bool retval;
    {
        LOCK_LIBDNF5();
        retval = session.read_all_repos();
    }
    auto reply = call.createReply();
    reply << retval;
    return reply;
}

sdbus::MethodReply Base::impl_clean(
    sdbus::MethodCall & call, const std::string & cache_type, const dnfdaemon::KeyValueMap & options) {
    bool interactive = dnfdaemon::key_value_map_get<bool>(options, "interactive", true);

    // let the "expire-cache" do anyone, just as read_all_repos()
    if (cache_type != "expire-cache" &&
        !session.check_authorization(
            dnfdaemon::POLKIT_EXECUTE_RPM_TRUSTED_TRANSACTION, call.getSender(), interactive)) {
        throw std::runtime_error("Not authorized");
    }

    bool success{false};
    std::string error_msg{};

    if (ALLOWED_CACHE_TYPES.find(cache_type) == ALLOWED_CACHE_TYPES.end()) {
        error_msg = fmt::format("Unsupported cache type to clean up: \"{}\".", cache_type);
    } else {
        auto base = session.get_base();
        std::filesystem::path cachedir{base->get_config().get_cachedir_option().get_value()};
        std::error_code ec;
        std::vector<std::string> remove_errs{};
        for (const auto & dir_entry : std::filesystem::directory_iterator(cachedir, ec)) {
            if (!dir_entry.is_directory()) {
                continue;
            }
            libdnf5::repo::RepoCache cache(*base, dir_entry.path());
            try {
                if (cache_type == "all") {
                    cache.remove_all();
                } else if (cache_type == "packages") {
                    cache.remove_packages();
                } else if (cache_type == "metadata") {
                    cache.remove_metadata();
                } else if (cache_type == "dbcache") {
                    cache.remove_solv_files();
                } else if (cache_type == "expire-cache") {
                    cache.write_attribute(libdnf5::repo::RepoCache::ATTRIBUTE_EXPIRED);
                }
            } catch (const std::exception & ex) {
                remove_errs.emplace_back(fmt::format(" - \"{0}\": {1}", dir_entry.path().native(), ex.what()));
            }
        }
        if (ec) {
            error_msg = fmt::format("Cannot iterate the cache directory: \"{}\".", cachedir.string());
        } else if (!remove_errs.empty()) {
            error_msg =
                fmt::format("Failed to cleanup repository cache:\n{}", libdnf5::utils::string::join(remove_errs, "\n"));
        } else {
            success = true;
        }
    }

    auto reply = call.createReply();
    reply << success;
    reply << error_msg;
    return reply;
}

sdbus::MethodReply Base::clean_with_options(sdbus::MethodCall & call) {
    std::string cache_type{};
    dnfdaemon::KeyValueMap options;
    call >> cache_type >> options;

    return impl_clean(call, cache_type, options);
}

sdbus::MethodReply Base::clean(sdbus::MethodCall & call) {
    std::string cache_type{};
    dnfdaemon::KeyValueMap options{};
    call >> cache_type;

    return impl_clean(call, cache_type, options);
}

sdbus::MethodReply Base::reset(sdbus::MethodCall & call) {
    bool success{true};
    std::string error_msg{};

    auto & transaction_mutex = session.get_transaction_mutex();
    if (!transaction_mutex.try_lock()) {
        success = false;
        error_msg = "Cannot reset, an rpm transaction is running.";
    } else {
        std::lock_guard<std::mutex> transaction_lock(transaction_mutex, std::adopt_lock);
        {
            LOCK_LIBDNF5();
            session.reset_base();
        }
    }
    auto reply = call.createReply();
    reply << success;
    reply << error_msg;
    return reply;
}
