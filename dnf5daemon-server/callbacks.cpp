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

#include "callbacks.hpp"

#include "dbus.hpp"
#include "session.hpp"
#include "transaction.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <string>

DbusCallback::DbusCallback(Session & session) : session(session) {
    dbus_object = session.get_dbus_object();
}

sdbus::Signal DbusCallback::create_signal(std::string interface, std::string signal_name) {
    auto signal = dbus_object->createSignal(interface, signal_name);
    // TODO(mblaha): uncomment once setDestination is available in sdbus-cpp
    //signal.setDestination(session->get_sender());
    signal << session.get_object_path();
    return signal;
}

std::chrono::time_point<std::chrono::steady_clock> DbusCallback::prev_print_time = std::chrono::steady_clock::now();


DbusPackageCB::DbusPackageCB(Session & session, const libdnf::rpm::Package & pkg)
    : DbusCallback(session),
      pkg_id(pkg.get_id().id) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_PACKAGE_DOWNLOAD_START);
        signal << pkg.get_full_nevra();
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

int DbusPackageCB::end(TransferStatus status, const char * msg) {
    try {
        // Due to is_time_to_print() timeout it is possible that progress signal was not
        // emitted with correct downloaded / total_to_download value - especially for small packages.
        // Emit the progress signal at least once signalling that 100% of the package was downloaded.
        if (status == TransferStatus::SUCCESSFUL) {
            auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_PACKAGE_DOWNLOAD_PROGRESS);
            signal << total;
            signal << total;
            dbus_object->emitSignal(signal);
        }
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_PACKAGE_DOWNLOAD_END);
        signal << static_cast<int>(status);
        signal << msg;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
    return 0;
}

int DbusPackageCB::progress(double total_to_download, double downloaded) {
    total = total_to_download;
    try {
        if (is_time_to_print()) {
            auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_PACKAGE_DOWNLOAD_PROGRESS);
            signal << downloaded;
            signal << total_to_download;
            dbus_object->emitSignal(signal);
        }
    } catch (...) {
    }
    return 0;
}

int DbusPackageCB::mirror_failure(const char * msg, const char * url) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_PACKAGE_DOWNLOAD_MIRROR_FAILURE);
        signal << msg;
        signal << url;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
    return 0;
}

sdbus::Signal DbusPackageCB::create_signal(std::string interface, std::string signal_name) {
    auto signal = DbusCallback::create_signal(interface, signal_name);
    signal << pkg_id;
    return signal;
}


void DbusRepoCB::start(const char * what) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_REPO, dnfdaemon::SIGNAL_REPO_LOAD_START);
        signal << what;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusRepoCB::end([[maybe_unused]] const char * error_message) {
    // TODO(lukash) forward the error message to the client?
    try {
        {
            auto signal = create_signal(dnfdaemon::INTERFACE_REPO, dnfdaemon::SIGNAL_REPO_LOAD_PROGRESS);
            signal << total;
            signal << total;
            dbus_object->emitSignal(signal);
        }
        {
            auto signal = create_signal(dnfdaemon::INTERFACE_REPO, dnfdaemon::SIGNAL_REPO_LOAD_END);
            dbus_object->emitSignal(signal);
        }
    } catch (...) {
    }
}

int DbusRepoCB::progress(double total_to_download, double downloaded) {
    total = static_cast<uint64_t>(total_to_download);
    try {
        if (is_time_to_print()) {
            auto signal = create_signal(dnfdaemon::INTERFACE_REPO, dnfdaemon::SIGNAL_REPO_LOAD_PROGRESS);
            signal << static_cast<uint64_t>(downloaded);
            signal << static_cast<uint64_t>(total_to_download);
            dbus_object->emitSignal(signal);
        }
    } catch (...) {
    }
    return 0;
}

bool DbusRepoCB::repokey_import(
    const std::string & id,
    const std::string & user_id,
    const std::string & fingerprint,
    const std::string & url,
    long int timestamp) {
    bool confirmed;
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_REPO, dnfdaemon::SIGNAL_REPO_KEY_IMPORT_REQUEST);
        signal << id << user_id << fingerprint << url << static_cast<int64_t>(timestamp);
        // wait for client's confirmation
        confirmed = session.wait_for_key_confirmation(id, signal);
    } catch (...) {
        confirmed = false;
    }
    return confirmed;
}


sdbus::Signal DbusTransactionCB::create_signal_pkg(
    std::string interface, std::string signal_name, const std::string & nevra) {
    auto signal = create_signal(interface, signal_name);
    signal << nevra;
    return signal;
}


void DbusTransactionCB::install_start(const libdnf::rpm::TransactionItem & item, uint64_t total) {
    try {
        dnfdaemon::RpmTransactionItemActions action;
        action = dnfdaemon::transaction_package_to_action(item);
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_ACTION_START, item.get_package().get_full_nevra());
        signal << static_cast<int>(action);
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::install_progress(const libdnf::rpm::TransactionItem & item, uint64_t amount, uint64_t total) {
    try {
        if (is_time_to_print()) {
            auto signal = create_signal_pkg(
                dnfdaemon::INTERFACE_RPM,
                dnfdaemon::SIGNAL_TRANSACTION_ACTION_PROGRESS,
                item.get_package().get_full_nevra());
            signal << amount;
            signal << total;
            dbus_object->emitSignal(signal);
        }
    } catch (...) {
    }
}

void DbusTransactionCB::install_stop(const libdnf::rpm::TransactionItem & item, uint64_t /*amount*/, uint64_t total) {
    try {
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_ACTION_STOP, item.get_package().get_full_nevra());
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}


void DbusTransactionCB::script_start(
    const libdnf::rpm::TransactionItem * /*item*/,
    libdnf::rpm::Nevra nevra,
    libdnf::rpm::TransactionCallbacks::ScriptType type) {
    try {
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_START, to_full_nevra_string(nevra));
        signal << static_cast<int>(type);
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::script_stop(
    const libdnf::rpm::TransactionItem * /*item*/,
    libdnf::rpm::Nevra nevra,
    libdnf::rpm::TransactionCallbacks::ScriptType type,
    uint64_t return_code) {
    try {
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_STOP, to_full_nevra_string(nevra));
        signal << static_cast<int>(type);
        signal << return_code;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::elem_progress(const libdnf::rpm::TransactionItem & item, uint64_t amount, uint64_t total) {
    try {
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_ELEM_PROGRESS, item.get_package().get_full_nevra());
        signal << amount;
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::script_error(
    const libdnf::rpm::TransactionItem * /*item*/,
    libdnf::rpm::Nevra nevra,
    libdnf::rpm::TransactionCallbacks::ScriptType type,
    uint64_t return_code) {
    try {
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_ERROR, to_full_nevra_string(nevra));
        signal << static_cast<int>(type);
        signal << return_code;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}


void DbusTransactionCB::transaction_start(uint64_t total) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_START);
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::transaction_progress(uint64_t amount, uint64_t total) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_PROGRESS);
        signal << amount;
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::transaction_stop(uint64_t total) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_STOP);
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}


void DbusTransactionCB::verify_start(uint64_t total) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_VERIFY_START);
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::verify_progress(uint64_t amount, uint64_t total) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_VERIFY_PROGRESS);
        signal << amount;
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::verify_stop(uint64_t total) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_VERIFY_STOP);
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}


void DbusTransactionCB::unpack_error(const libdnf::rpm::TransactionItem & item) {
    try {
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_UNPACK_ERROR, item.get_package().get_full_nevra());
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::finish() {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_FINISHED);
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}
