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

#ifndef DNF5DAEMON_SERVER_CALLBACKS_HPP
#define DNF5DAEMON_SERVER_CALLBACKS_HPP

#include <libdnf5/repo/download_callbacks.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/repo/repo_callbacks.hpp>
#include <libdnf5/rpm/rpm_signature.hpp>
#include <libdnf5/rpm/transaction_callbacks.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <string>

class Session;

namespace dnf5daemon {


struct DownloadUserData {
    std::string download_id{};
};

class DbusCallback {
public:
    explicit DbusCallback(Session & session);
    virtual ~DbusCallback() = default;

protected:
    static std::chrono::time_point<std::chrono::steady_clock> prev_print_time;
    Session & session;
    sdbus::IObject * dbus_object;

    virtual sdbus::Signal create_signal(std::string interface, std::string signal_name);
    static bool is_time_to_print() {
        auto now = std::chrono::steady_clock::now();
        auto delta = now - prev_print_time;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
        if (ms > 400) {
            prev_print_time = now;
            return true;
        }
        return false;
    }
};


class DownloadCB : public DbusCallback, public libdnf5::repo::DownloadCallbacks {
public:
    DownloadCB(Session & session) : DbusCallback(session), libdnf5::repo::DownloadCallbacks() {}

    void * add_new_download(void * user_data, const char * description, double total_to_download) override;

    int progress(void * user_cb_data, double total_to_download, double downloaded) override;

    int end(void * user_cb_data, TransferStatus status, const char * msg) override;

    int mirror_failure(void * user_cb_data, const char * msg, const char * url, const char * metadata) override;

private:
    sdbus::Signal create_signal_download(const std::string & signal_name, void * user_data);
};


class KeyImportRepoCB : public DbusCallback, public libdnf5::repo::RepoCallbacks {
public:
    KeyImportRepoCB(Session & session) : DbusCallback(session) {}

    bool repokey_import(const libdnf5::rpm::KeyInfo & key_info) override;
};


class DbusTransactionCB : public libdnf5::rpm::TransactionCallbacks, public DbusCallback {
public:
    explicit DbusTransactionCB(Session & session) : DbusCallback(session) {}
    virtual ~DbusTransactionCB() = default;

    // overall transaction progress
    void before_begin(uint64_t total) override;
    void after_complete(bool success) override;

    // transaction preparation
    void transaction_start(uint64_t total) override;
    void transaction_progress(uint64_t amount, uint64_t total) override;
    void transaction_stop(uint64_t total) override;

    // install a package
    void install_start(const libdnf5::rpm::TransactionItem & item, uint64_t) override;
    void install_progress(const libdnf5::rpm::TransactionItem & item, uint64_t amount, uint64_t total) override;
    void install_stop(const libdnf5::rpm::TransactionItem & item, uint64_t amount, uint64_t total) override;

    // uninstall a package (the same messages as for install are used)
    void uninstall_start(const libdnf5::rpm::TransactionItem & item, uint64_t total) override {
        install_start(item, total);
    }
    void uninstall_progress(const libdnf5::rpm::TransactionItem & item, uint64_t amount, uint64_t total) override {
        install_progress(item, amount, total);
    }
    void uninstall_stop(const libdnf5::rpm::TransactionItem & item, uint64_t amount, uint64_t total) override {
        install_stop(item, amount, total);
    }

    void script_start(
        const libdnf5::rpm::TransactionItem * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type) override;
    void script_stop(
        const libdnf5::rpm::TransactionItem * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type,
        uint64_t return_code) override;
    void script_error(
        const libdnf5::rpm::TransactionItem * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type,
        uint64_t return_code) override;

    void elem_progress(const libdnf5::rpm::TransactionItem & item, uint64_t amount, uint64_t total) override;

    void verify_start(uint64_t total) override;
    void verify_progress(uint64_t amount, uint64_t total) override;
    void verify_stop(uint64_t total) override;

    void unpack_error(const libdnf5::rpm::TransactionItem & item) override;
    void cpio_error(const libdnf5::rpm::TransactionItem & /*item*/) override{};

    // whole rpm transaction is finished
    void finish();

private:
    sdbus::Signal create_signal_pkg(std::string interface, std::string signal_name, const std::string & nevra);
};

}  // namespace dnf5daemon

#endif
