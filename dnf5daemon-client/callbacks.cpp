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

#include "context.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <dnf5daemon-server/transaction.hpp>
#include <libdnf5-cli/tty.hpp>
#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/repo/download_callbacks.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <string>

namespace dnfdaemon::client {

DbusCallback::DbusCallback(Context & context, sdbus::IConnection & connection)
    : session_object_path(context.get_session_object_path()) {
    session_proxy = sdbus::createProxy(connection, dnfdaemon::DBUS_NAME, session_object_path);
}

bool DbusCallback::signature_valid(sdbus::Signal & signal) {
    // check that signal is emitted by the correct session object
    sdbus::ObjectPath object_path;
    signal >> object_path;
    return object_path == session_object_path;
}


DownloadCB::DownloadCB(Context & context, sdbus::IConnection & connection)
    : DbusCallback(context, connection),
      assume_yes(context.get_assumeyes_option()),
      assume_no(context.get_assumeno_option()),
      default_yes(context.get_defaultyes_option()) {}

void DownloadCB::register_signals() {
    // register signal handlers
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_DOWNLOAD_ADD_NEW, [this](sdbus::Signal signal) -> void {
            this->add_new_download(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_DOWNLOAD_END, [this](sdbus::Signal signal) -> void {
            this->end(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_DOWNLOAD_PROGRESS, [this](sdbus::Signal signal) -> void {
            this->progress(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_DOWNLOAD_MIRROR_FAILURE, [this](sdbus::Signal signal) -> void {
            this->mirror_failure(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_KEY_IMPORT_REQUEST, [this](sdbus::Signal signal) -> void {
            this->key_import(signal);
        });
#ifndef SDBUS_CPP_VERSION_2
    session_proxy->finishRegistration();
#endif
}


libdnf5::cli::progressbar::DownloadProgressBar * DownloadCB::find_progress_bar(const std::string & download_id) {
    auto iter = progress_bars.find(download_id);
    if (iter != progress_bars.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
}


void DownloadCB::print() {
    if (multi_progress_bar) {
        multi_progress_bar->print();
        printed = true;
    }
}


void DownloadCB::reset_progress_bar() {
    multi_progress_bar.reset();
    if (printed) {
        printed = false;
    }
}

void DownloadCB::set_number_widget_visible(bool value) {
    number_widget_visible = value;
}

void DownloadCB::set_show_total_bar_limit(std::size_t limit) {
    show_total_bar_limit = limit;
    if (multi_progress_bar) {
        multi_progress_bar->set_total_bar_visible_limit(limit);
    }
}

void DownloadCB::add_new_download(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        std::string download_id;
        std::string description;
        int64_t total_to_download;
        signal >> download_id;
        signal >> description;
        signal >> total_to_download;
        if (!multi_progress_bar) {
            multi_progress_bar = std::make_unique<libdnf5::cli::progressbar::MultiProgressBar>();
            multi_progress_bar->set_total_bar_visible_limit(show_total_bar_limit);
        }
        auto progress_bar = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(
            total_to_download > 0 ? total_to_download : -1, description);
        progress_bar->set_auto_finish(false);
        progress_bars.emplace(download_id, progress_bar.get());
        multi_progress_bar->add_bar(std::move(progress_bar));
    }
}


void DownloadCB::end(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        std::string download_id;
        signal >> download_id;
        auto progress_bar = find_progress_bar(download_id);
        if (progress_bar == nullptr) {
            return;
        }

        unsigned int status_i;
        std::string msg;
        signal >> status_i;
        signal >> msg;
        auto status = static_cast<libdnf5::repo::DownloadCallbacks::TransferStatus>(status_i);
        switch (status) {
            case libdnf5::repo::DownloadCallbacks::TransferStatus::SUCCESSFUL:
                progress_bar->set_total_ticks(progress_bar->get_ticks());
                progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
                break;
            case libdnf5::repo::DownloadCallbacks::TransferStatus::ALREADYEXISTS:
                // skipping the download -> downloading 0 bytes
                progress_bar->set_ticks(0);
                progress_bar->set_total_ticks(0);
                progress_bar->add_message(libdnf5::cli::progressbar::MessageType::SUCCESS, msg);
                progress_bar->start();
                progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
                break;
            case libdnf5::repo::DownloadCallbacks::TransferStatus::ERROR:
                progress_bar->add_message(libdnf5::cli::progressbar::MessageType::ERROR, msg);
                progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::ERROR);
                break;
        }
        print();
    }
}

void DownloadCB::progress(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        std::string download_id;
        signal >> download_id;
        auto progress_bar = find_progress_bar(download_id);
        if (progress_bar == nullptr) {
            return;
        }
        int64_t total_to_download;
        int64_t downloaded;
        signal >> total_to_download;
        signal >> downloaded;
        if (total_to_download > 0) {
            progress_bar->set_total_ticks(total_to_download);
        }
        if (progress_bar->get_state() == libdnf5::cli::progressbar::ProgressBarState::READY) {
            progress_bar->start();
        }
        progress_bar->set_ticks(downloaded);
        print();
    }
}

void DownloadCB::mirror_failure(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        std::string download_id;
        signal >> download_id;
        auto progress_bar = find_progress_bar(download_id);
        if (progress_bar == nullptr) {
            return;
        }
        std::string msg;
        std::string url;
        std::string metadata;
        signal >> msg;
        signal >> url;
        signal >> metadata;

        std::string message = msg + " - " + url;
        if (!metadata.empty()) {
            message += " - " + metadata;
        }
        progress_bar->add_message(libdnf5::cli::progressbar::MessageType::ERROR, message);
        print();
    }
}

void DownloadCB::key_import(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        std::string key_id;
        std::vector<std::string> user_ids;
        std::string fingerprint;
        std::string url;
        signal >> key_id >> user_ids >> fingerprint >> url;

        std::cerr << std::endl << "Importing OpenPGP key 0x" + key_id << ":\n";
        for (auto & user_id : user_ids) {
            std::cerr << " Userid     : \"" + user_id << "\"\n";
        }
        std::cerr << " Fingerprint: " + fingerprint << std::endl;
        std::cerr << " From       : " + url << std::endl;

        // ask user for the key import confirmation
        auto confirmed = libdnf5::cli::utils::userconfirm::userconfirm(*this);

        // signal the confirmation back to the server
        try {
            session_proxy->callMethod("confirm_key")
                .onInterface(dnfdaemon::INTERFACE_REPO)
                .withTimeout(static_cast<uint64_t>(-1))
                .withArguments(key_id, confirmed);
        } catch (const sdbus::Error & ex) {
            std::cerr << ex.what() << std::endl;
        }
    }
}

TransactionCB::TransactionCB(Context & context, sdbus::IConnection & connection) : DbusCallback(context, connection) {}


void TransactionCB::register_signals() {
    // register signal handlers
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_VERIFY_START, [this](sdbus::Signal signal) -> void {
            this->verify_start(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_VERIFY_PROGRESS, [this](sdbus::Signal signal) -> void {
            this->verify_progress(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_VERIFY_STOP, [this](sdbus::Signal signal) -> void {
            this->verify_end(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_START,
        [this](sdbus::Signal signal) -> void { this->transaction_start(signal); });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_PROGRESS,
        [this](sdbus::Signal signal) -> void { this->transaction_progress(signal); });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_STOP, [this](sdbus::Signal signal) -> void {
            this->transaction_end(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_ACTION_START, [this](sdbus::Signal signal) -> void {
            this->action_start(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_ACTION_PROGRESS, [this](sdbus::Signal signal) -> void {
            this->action_progress(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_ACTION_STOP, [this](sdbus::Signal signal) -> void {
            this->action_end(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_START, [this](sdbus::Signal signal) -> void {
            this->script_start(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_STOP, [this](sdbus::Signal signal) -> void {
            this->script_stop(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_ERROR, [this](sdbus::Signal signal) -> void {
            this->script_error(signal);
        });
    session_proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_FINISHED, [this](sdbus::Signal signal) -> void {
            this->finished(signal);
        });
#ifndef SDBUS_CPP_VERSION_2
    session_proxy->finishRegistration();
#endif
}

void TransactionCB::new_progress_bar(uint64_t total, const std::string & description) {
    if (active_progress_bar && active_progress_bar->get_state() != libdnf5::cli::progressbar::ProgressBarState::ERROR) {
        active_progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
    }
    auto progress_bar =
        std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(static_cast<int64_t>(total), description);
    progress_bar->set_auto_finish(false);
    progress_bar->start();
    active_progress_bar = progress_bar.get();
    multi_progress_bar.add_bar(std::move(progress_bar));
}

void TransactionCB::verify_start(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        uint64_t total;
        signal >> total;
        new_progress_bar(total, "Verify package files");
    }
}

void TransactionCB::verify_progress(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        uint64_t amount;
        signal >> amount;

        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        multi_progress_bar.print();
    }
}

void TransactionCB::verify_end(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        uint64_t total;
        signal >> total;
        active_progress_bar->set_ticks(static_cast<int64_t>(total));
        multi_progress_bar.print();
    }
}

void TransactionCB::transaction_start(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        uint64_t total;
        signal >> total;
        new_progress_bar(total, "Prepare transaction");
    }
}

void TransactionCB::transaction_progress(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        uint64_t amount;
        signal >> amount;

        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        multi_progress_bar.print();
    }
}

void TransactionCB::transaction_end(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        uint64_t total;
        signal >> total;
        active_progress_bar->set_ticks(static_cast<int64_t>(total));
        multi_progress_bar.print();
    }
}

void TransactionCB::action_start(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        std::string nevra;
        unsigned int action_i;
        uint64_t total;
        signal >> nevra;
        signal >> action_i;
        signal >> total;
        auto action = static_cast<dnfdaemon::RpmTransactionItemActions>(action_i);
        std::string msg;
        switch (action) {
            case dnfdaemon::RpmTransactionItemActions::INSTALL:
                msg = "Installing";
                break;
            case dnfdaemon::RpmTransactionItemActions::ERASE:
                msg = "Removing";
                break;
            case dnfdaemon::RpmTransactionItemActions::DOWNGRADE:
                msg = "Downgrading";
                break;
            case dnfdaemon::RpmTransactionItemActions::UPGRADE:
                msg = "Upgrading";
                break;
            case dnfdaemon::RpmTransactionItemActions::REINSTALL:
                msg = "Reinstalling";
                break;
            case dnfdaemon::RpmTransactionItemActions::CLEANUP:
                msg = "Cleanup";
                break;
        }
        new_progress_bar(total, msg + " " + nevra);
        multi_progress_bar.print();
    }
}

void TransactionCB::action_progress(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        std::string nevra;
        uint64_t amount;
        signal >> nevra;
        signal >> amount;

        active_progress_bar->set_ticks(static_cast<int64_t>(amount));
        multi_progress_bar.print();
    }
}

void TransactionCB::action_end(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        [[maybe_unused]] std::string nevra;
        uint64_t total;
        signal >> nevra;
        signal >> total;
        active_progress_bar->set_ticks(static_cast<int64_t>(total));
        multi_progress_bar.print();
    }
}

void TransactionCB::script_start(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        std::string nevra;
        signal >> nevra;
        active_progress_bar->add_message(libdnf5::cli::progressbar::MessageType::INFO, "Running scriptlet: " + nevra);
        multi_progress_bar.print();
    }
}

void TransactionCB::script_stop(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        std::string nevra;
        signal >> nevra;
        active_progress_bar->add_message(libdnf5::cli::progressbar::MessageType::INFO, "Stop scriptlet: " + nevra);
        multi_progress_bar.print();
    }
}

void TransactionCB::script_error(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        std::string nevra;
        signal >> nevra;
        active_progress_bar->add_message(libdnf5::cli::progressbar::MessageType::ERROR, "Error in scriptlet: " + nevra);
        multi_progress_bar.print();
    }
}

void TransactionCB::unpack_error(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        std::string nevra;
        signal >> nevra;
        active_progress_bar->add_message(libdnf5::cli::progressbar::MessageType::ERROR, "Unpack error: " + nevra);
        multi_progress_bar.print();
    }
}

void TransactionCB::finished(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        if (active_progress_bar &&
            active_progress_bar->get_state() != libdnf5::cli::progressbar::ProgressBarState::ERROR) {
            active_progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
        }
        multi_progress_bar.print();
    }
}
}  // namespace dnfdaemon::client
