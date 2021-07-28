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

#ifndef DNFDAEMON_SERVER_SESSION_HPP
#define DNFDAEMON_SERVER_SESSION_HPP

#include "dbus.hpp"
#include "threads_manager.hpp"
#include "utils.hpp"

#include <libdnf/base/base.hpp>
#include <libdnf/base/goal.hpp>
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
    explicit IDbusSessionService(Session & session) : session(session){};
    virtual ~IDbusSessionService() = default;
    virtual void dbus_register() = 0;

protected:
    Session & session;
};

class Session {
public:
    Session(
        sdbus::IConnection & connection,
        dnfdaemon::KeyValueMap session_configuration,
        std::string object_path,
        std::string sender);
    ~Session();

    template <typename ItemType>
    ItemType session_configuration_value(const std::string & key, const ItemType & default_value) {
        return key_value_map_get(session_configuration, key, default_value);
    }
    template <typename ItemType>
    ItemType session_configuration_value(const std::string & key) {
        return key_value_map_get<ItemType>(session_configuration, key);
    }

    std::string get_object_path() { return object_path; };
    sdbus::IConnection & get_connection() { return connection; };
    libdnf::Base * get_base() { return base.get(); };
    ThreadsManager & get_threads_manager() { return threads_manager; };
    sdbus::IObject * get_dbus_object() { return dbus_object.get(); };
    libdnf::Goal & get_goal() { return goal; };
    libdnf::base::Transaction * get_transaction() { return transaction.get(); };
    void set_transaction(const libdnf::base::Transaction & src) {
        transaction.reset(new libdnf::base::Transaction(src));
    };
    std::string get_sender() const { return sender; };

    bool check_authorization(const std::string & actionid, const std::string & sender);
    void fill_sack();
    bool read_all_repos();
    std::optional<std::string> session_locale;
    void confirm_key(const std::string & key_id, const bool confirmed);
    bool wait_for_key_confirmation(const std::string & key_id, sdbus::Signal & signal);

private:
    sdbus::IConnection & connection;
    std::unique_ptr<libdnf::Base> base;
    libdnf::Goal goal;
    std::unique_ptr<libdnf::base::Transaction> transaction{nullptr};
    dnfdaemon::KeyValueMap session_configuration;
    std::string object_path;
    std::vector<std::unique_ptr<IDbusSessionService>> services{};
    ThreadsManager threads_manager;
    std::atomic<dnfdaemon::RepoStatus> repositories_status{dnfdaemon::RepoStatus::NOT_READY};
    std::unique_ptr<sdbus::IObject> dbus_object;
    std::string sender;
    // repository key import confirmation
    enum class KeyConfirmationStatus { PENDING, CONFIRMED, REJECTED };
    std::mutex key_import_mutex;
    std::condition_variable key_import_condition;
    std::map<std::string, KeyConfirmationStatus> key_import_status{};  // map key_id: confirmation status
};

#endif
