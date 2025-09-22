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

#ifndef DNF5DAEMON_SERVER_SESSION_HPP
#define DNF5DAEMON_SERVER_SESSION_HPP

#include "dbus.hpp"
#include "threads_manager.hpp"
#include "utils.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/base/goal.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Session;

class IDbusSessionService {
public:
    explicit IDbusSessionService(Session & session) : session(session) {};
    virtual ~IDbusSessionService() = default;
    virtual void dbus_register() = 0;

    Session & get_session() const noexcept { return session; }

protected:
    Session & session;
};

class Session {
public:
    enum class CancelDownload { NOT_RUNNING, NOT_REQUESTED, REQUESTED, NOT_ALLOWED };
    Session(
        sdbus::IConnection & connection,
        dnfdaemon::KeyValueMap session_configuration,
        const sdbus::ObjectPath & object_path,
        const std::string & sender);
    ~Session();

    template <typename ItemType>
    ItemType session_configuration_value(const std::string & key, const ItemType & default_value) {
        return dnfdaemon::key_value_map_get(session_configuration, key, default_value);
    }
    template <typename ItemType>
    ItemType session_configuration_value(const std::string & key) {
        return dnfdaemon::key_value_map_get<ItemType>(session_configuration, key);
    }

    const sdbus::ObjectPath & get_object_path() { return object_path; };
    sdbus::IConnection & get_connection() { return connection; };
    libdnf5::Base * get_base() { return base.get(); };
    ThreadsManager & get_threads_manager() { return threads_manager; };
    sdbus::IObject * get_dbus_object() { return dbus_object.get(); };
    libdnf5::Goal & get_goal() { return *goal; };
    libdnf5::base::Transaction * get_transaction() { return transaction.get(); };
    void set_transaction(const libdnf5::base::Transaction & src) {
        transaction.reset(new libdnf5::base::Transaction(src));
    };
    std::string get_sender() const { return sender; };

    bool check_authorization(
        const std::string & actionid, const std::string & sender, bool allow_user_interaction = true);
    void fill_sack();
    bool read_all_repos();
    std::optional<std::string> session_locale;
    void confirm_key(const std::string & key_id, const bool confirmed);
    bool wait_for_key_confirmation(const std::string & key_id, sdbus::Signal & signal);
    /// download packages for the current transaction
    void download_transaction_packages();
    /// prepare the current transaction to run during the next reboot
    void store_transaction_offline(bool downloadonly);

    /// Getter for download cancel request flag.
    CancelDownload get_cancel_download() { return cancel_download.load(); }
    /// Setter for download cancel request flag.
    void set_cancel_download(CancelDownload value) { cancel_download.store(value); }

    std::mutex & get_transaction_mutex() { return transaction_mutex; }
    std::mutex & get_libdnf5_mutex() { return libdnf5_mutex; }

    void reset_goal();
    void reset_base();

private:
    void setup_base();

    sdbus::IConnection & connection;
    std::unique_ptr<libdnf5::Base> base;
    std::unique_ptr<libdnf5::Goal> goal;
    std::unique_ptr<libdnf5::base::Transaction> transaction{nullptr};
    dnfdaemon::KeyValueMap session_configuration;
    sdbus::ObjectPath object_path;
    std::vector<std::unique_ptr<IDbusSessionService>> services{};
    ThreadsManager threads_manager;
    std::atomic<dnfdaemon::RepoStatus> repositories_status{dnfdaemon::RepoStatus::NOT_READY};
    std::unique_ptr<sdbus::IObject> dbus_object;
    std::string sender;
    // repository key import confirmation
    enum class KeyConfirmationStatus { PENDING, CONFIRMED, REJECTED };
    std::mutex key_import_mutex;
    std::mutex transaction_mutex;
    // Mutex to serialize D-Bus method calls within the same session. Needed
    // because libdnf5 and libsolv are not thread-safe.
    std::mutex libdnf5_mutex;
    std::condition_variable key_import_condition;
    std::map<std::string, KeyConfirmationStatus> key_import_status{};  // map key_id: confirmation status
    std::atomic<CancelDownload> cancel_download{CancelDownload::NOT_RUNNING};
};

#endif
