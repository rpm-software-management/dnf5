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

namespace dnf5daemon {


DbusCallback::DbusCallback(Session & session) : session(session) {
    dbus_object = session.get_dbus_object();
}

sdbus::Signal DbusCallback::create_signal(std::string interface, std::string signal_name) {
    auto signal = dbus_object->createSignal(interface, signal_name);
    signal.setDestination(session.get_sender());
    signal << session.get_object_path();
    return signal;
}

std::chrono::time_point<std::chrono::steady_clock> DbusCallback::prev_print_time = std::chrono::steady_clock::now();


sdbus::Signal DownloadCB::create_signal_download(const std::string & signal_name, void * user_data) {
    auto signal = create_signal(dnfdaemon::INTERFACE_BASE, signal_name);
    if (user_data) {
        auto * data = reinterpret_cast<DownloadUserData *>(user_data);
        signal << data->download_id;
    }
    return signal;
}

void * DownloadCB::add_new_download(void * user_data, const char * description, double total_to_download) {
    try {
        auto signal = create_signal_download(dnfdaemon::SIGNAL_DOWNLOAD_ADD_NEW, user_data);
        signal << description;
        signal << static_cast<int64_t>(total_to_download);
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
    return user_data;
}

int DownloadCB::progress(void * user_cb_data, double total_to_download, double downloaded) {
    try {
        if (is_time_to_print()) {
            auto signal = create_signal_download(dnfdaemon::SIGNAL_DOWNLOAD_PROGRESS, user_cb_data);
            signal << static_cast<int64_t>(total_to_download);
            signal << static_cast<int64_t>(downloaded);
            dbus_object->emitSignal(signal);
        }
    } catch (...) {
    }
    return ReturnCode::OK;
}

int DownloadCB::end(void * user_cb_data, TransferStatus status, const char * msg) {
    try {
        auto signal = create_signal_download(dnfdaemon::SIGNAL_DOWNLOAD_END, user_cb_data);
        signal << static_cast<uint32_t>(status);
        signal << msg;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
    return ReturnCode::OK;
}

int DownloadCB::mirror_failure(void * user_cb_data, const char * msg, const char * url, const char * metadata) {
    try {
        auto signal = create_signal_download(dnfdaemon::SIGNAL_DOWNLOAD_MIRROR_FAILURE, user_cb_data);
        signal << msg;
        signal << url;
        signal << metadata;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
    return ReturnCode::OK;
}


bool KeyImportRepoCB::repokey_import(const libdnf5::rpm::KeyInfo & key_info) {
    bool confirmed;
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_KEY_IMPORT_REQUEST);
        signal << key_info.get_short_key_id() << key_info.get_user_ids() << key_info.get_fingerprint()
               << key_info.get_url() << static_cast<int64_t>(key_info.get_timestamp());
        // wait for client's confirmation
        confirmed = session.wait_for_key_confirmation(key_info.get_short_key_id(), signal);
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


void DbusTransactionCB::before_begin(uint64_t total) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_BEFORE_BEGIN);
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::after_complete(bool success) {
    try {
        auto signal = create_signal(dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_AFTER_COMPLETE);
        signal << success;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::install_start(const libdnf5::rpm::TransactionItem & item, uint64_t total) {
    try {
        dnfdaemon::RpmTransactionItemActions action;
        action = dnfdaemon::transaction_package_to_action(item);
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_ACTION_START, item.get_package().get_full_nevra());
        signal << static_cast<uint32_t>(action);
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::install_progress(const libdnf5::rpm::TransactionItem & item, uint64_t amount, uint64_t total) {
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

void DbusTransactionCB::install_stop(const libdnf5::rpm::TransactionItem & item, uint64_t /*amount*/, uint64_t total) {
    try {
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_ACTION_STOP, item.get_package().get_full_nevra());
        signal << total;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}


void DbusTransactionCB::script_start(
    const libdnf5::rpm::TransactionItem * /*item*/,
    libdnf5::rpm::Nevra nevra,
    libdnf5::rpm::TransactionCallbacks::ScriptType type) {
    try {
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_START, to_full_nevra_string(nevra));
        signal << static_cast<uint32_t>(type);
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::script_stop(
    const libdnf5::rpm::TransactionItem * /*item*/,
    libdnf5::rpm::Nevra nevra,
    libdnf5::rpm::TransactionCallbacks::ScriptType type,
    uint64_t return_code) {
    try {
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_STOP, to_full_nevra_string(nevra));
        signal << static_cast<uint32_t>(type);
        signal << return_code;
        dbus_object->emitSignal(signal);
    } catch (...) {
    }
}

void DbusTransactionCB::elem_progress(const libdnf5::rpm::TransactionItem & item, uint64_t amount, uint64_t total) {
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
    const libdnf5::rpm::TransactionItem * /*item*/,
    libdnf5::rpm::Nevra nevra,
    libdnf5::rpm::TransactionCallbacks::ScriptType type,
    uint64_t return_code) {
    try {
        auto signal = create_signal_pkg(
            dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_ERROR, to_full_nevra_string(nevra));
        signal << static_cast<uint32_t>(type);
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


void DbusTransactionCB::unpack_error(const libdnf5::rpm::TransactionItem & item) {
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

}  // namespace dnf5daemon
