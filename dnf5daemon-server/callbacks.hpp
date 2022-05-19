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

#include <libdnf/repo/package_downloader.hpp>
#include <libdnf/repo/repo_callbacks.hpp>
#include <libdnf/rpm/transaction_callbacks.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <string>

class Session;

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

class DbusPackageCB : public libdnf::repo::DownloadCallbacks, public DbusCallback {
public:
    explicit DbusPackageCB(Session & session, const libdnf::rpm::Package & pkg);
    virtual ~DbusPackageCB() = default;

    int end(TransferStatus status, const char * msg) override;
    int progress(double total_to_download, double downloaded) override;
    int mirror_failure(const char * msg, const char * url) override;

private:
    int pkg_id;
    double total = 0;

    sdbus::Signal create_signal(std::string interface, std::string signal_name) override;
};

class DbusRepoCB : public libdnf::repo::RepoCallbacks, public DbusCallback {
public:
    explicit DbusRepoCB(Session & session) : DbusCallback(session) {}
    virtual ~DbusRepoCB() = default;

    void start(const char * what) override;
    void end(const char * error_message) override;
    int progress(double total_to_download, double downloaded) override;

    bool repokey_import(
        const std::string & id,
        const std::string & user_id,
        const std::string & fingerprint,
        const std::string & url,
        long int timestamp) override;

private:
    uint64_t total = 0;
};


class DbusTransactionCB : public libdnf::rpm::TransactionCallbacks, public DbusCallback {
public:
    explicit DbusTransactionCB(Session & session) : DbusCallback(session) {}
    virtual ~DbusTransactionCB() = default;

    // transaction preparation
    void transaction_start(uint64_t total) override;
    void transaction_progress(uint64_t amount, uint64_t total) override;
    void transaction_stop(uint64_t total) override;

    // install a package
    void install_start(const libdnf::rpm::TransactionItem & item, uint64_t) override;
    void install_progress(const libdnf::rpm::TransactionItem & item, uint64_t amount, uint64_t total) override;
    void install_stop(const libdnf::rpm::TransactionItem & item, uint64_t amount, uint64_t total) override;

    // uninstall a package (the same messages as for install are used)
    void uninstall_start(const libdnf::rpm::TransactionItem & item, uint64_t total) override {
        install_start(item, total);
    }
    void uninstall_progress(const libdnf::rpm::TransactionItem & item, uint64_t amount, uint64_t total) override {
        install_progress(item, amount, total);
    }
    void uninstall_stop(const libdnf::rpm::TransactionItem & item, uint64_t amount, uint64_t total) override {
        install_stop(item, amount, total);
    }

    void script_start(
        const libdnf::rpm::TransactionItem * item,
        libdnf::rpm::Nevra nevra,
        libdnf::rpm::TransactionCallbacks::ScriptType type) override;
    void script_stop(
        const libdnf::rpm::TransactionItem * item,
        libdnf::rpm::Nevra nevra,
        libdnf::rpm::TransactionCallbacks::ScriptType type,
        uint64_t return_code) override;
    void script_error(
        const libdnf::rpm::TransactionItem * item,
        libdnf::rpm::Nevra nevra,
        libdnf::rpm::TransactionCallbacks::ScriptType type,
        uint64_t return_code) override;

    void elem_progress(const libdnf::rpm::TransactionItem & item, uint64_t amount, uint64_t total) override;

    void verify_start(uint64_t total) override;
    void verify_progress(uint64_t amount, uint64_t total) override;
    void verify_stop(uint64_t total) override;

    void unpack_error(const libdnf::rpm::TransactionItem & item) override;
    void cpio_error(const libdnf::rpm::TransactionItem & /*item*/) override{};

    // whole rpm transaction is finished
    void finish();

private:
    sdbus::Signal create_signal_pkg(std::string interface, std::string signal_name, const std::string & nevra);
};

#endif
