/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "callbacks.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <dnfdaemon-server/transaction.hpp>
#include <libdnf-cli/utils/tty.hpp>
#include <libdnf/rpm/repo.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <string>

namespace dnfdaemon::client {


bool DbusCallback::signature_valid(sdbus::Signal & signal) {
    // check that signal is emited by the correct session object
    std::string object_path;
    signal >> object_path;
    return object_path == session_object_path;
}


RepoCB::RepoCB(sdbus::IProxy * proxy, std::string session_object_path) : DbusCallback(proxy, session_object_path) {
    // register signal handlers
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_START, [this](sdbus::Signal & signal) -> void {
            this->start(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_END, [this](sdbus::Signal & signal) -> void {
            this->end(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_BASE, dnfdaemon::SIGNAL_REPO_LOAD_PROGRESS, [this](sdbus::Signal & signal) -> void {
            this->progress(signal);
        });
}


void RepoCB::print_progress_bar() {
    if (libdnf::cli::utils::tty::is_interactive()) {
        std::cout << libdnf::cli::utils::tty::clear_line;
        for (std::size_t i = 0; i < msg_lines; i++) {
            std::cout << libdnf::cli::utils::tty::cursor_up << libdnf::cli::utils::tty::clear_line;
        }
        std::cout << "\r";
    }
    std::cout << progress_bar << std::flush;
    msg_lines = progress_bar.get_messages().size();
}

void RepoCB::start(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        std::string what;
        signal >> what;
        progress_bar.reset();
        progress_bar.set_description(what);
        progress_bar.set_auto_finish(false);
        progress_bar.start();
    }
}

void RepoCB::end(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        progress_bar.set_ticks(progress_bar.get_total_ticks());
        progress_bar.set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
        print_progress_bar();
        std::cout << std::endl;
    }
}

void RepoCB::progress(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        double downloaded;
        double total_to_download;
        signal >> downloaded;
        signal >> total_to_download;
        progress_bar.set_total_ticks(static_cast<int64_t>(total_to_download));
        progress_bar.set_ticks(static_cast<int64_t>(downloaded));
        print_progress_bar();
    }
}


PackageDownloadCB::PackageDownloadCB(sdbus::IProxy * proxy, std::string session_object_path)
    : DbusCallback(proxy, session_object_path) {
    // register signal handlers
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_PACKAGE_DOWNLOAD_START, [this](sdbus::Signal & signal) -> void {
            this->start(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_PACKAGE_DOWNLOAD_END, [this](sdbus::Signal & signal) -> void {
            this->end(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_PACKAGE_DOWNLOAD_PROGRESS, [this](sdbus::Signal & signal) -> void {
            this->progress(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_PACKAGE_DOWNLOAD_MIRROR_FAILURE,
        [this](sdbus::Signal & signal) -> void { this->mirror_failure(signal); });
}


void PackageDownloadCB::start(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        int pkg_id;
        std::string nevra;
        signal >> pkg_id;
        signal >> nevra;
        auto progress_bar = std::make_unique<libdnf::cli::progressbar::DownloadProgressBar>(-1, nevra);
        multi_progress_bar.add_bar(progress_bar.get());
        package_bars.emplace(pkg_id, std::move(progress_bar));
    }
}

void PackageDownloadCB::end(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        int pkg_id;
        signal >> pkg_id;
        auto progress_bar = find_progress_bar(pkg_id);
        if (progress_bar == nullptr) {
            return;
        }
        int status_i;
        signal >> status_i;
        std::string msg;
        signal >> msg;
        using namespace libdnf::rpm;
        auto status = static_cast<PackageTargetCB::TransferStatus>(status_i);
        switch (status) {
            case PackageTargetCB::TransferStatus::SUCCESSFUL:
                progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
                break;
            case PackageTargetCB::TransferStatus::ALREADYEXISTS:
                // skipping the download -> downloading 0 bytes
                progress_bar->set_ticks(0);
                progress_bar->set_total_ticks(0);
                progress_bar->add_message(libdnf::cli::progressbar::MessageType::SUCCESS, msg);
                progress_bar->start();
                progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
                break;
            case PackageTargetCB::TransferStatus::ERROR:
                progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, msg);
                progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
                break;
        }
        multi_progress_bar.print();
    }
}

void PackageDownloadCB::progress(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        int pkg_id;
        signal >> pkg_id;
        auto progress_bar = find_progress_bar(pkg_id);
        if (progress_bar == nullptr) {
            return;
        }
        double downloaded;
        double total_to_download;
        signal >> downloaded;
        signal >> total_to_download;
        if (total_to_download > 0) {
            progress_bar->set_total_ticks(static_cast<int64_t>(total_to_download));
        }
        if (progress_bar->get_state() == libdnf::cli::progressbar::ProgressBarState::READY) {
            progress_bar->start();
        }
        progress_bar->set_ticks(static_cast<int64_t>(downloaded));
        multi_progress_bar.print();
    }
}

void PackageDownloadCB::mirror_failure(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        int pkg_id;
        signal >> pkg_id;
        auto progress_bar = find_progress_bar(pkg_id);
        if (progress_bar == nullptr) {
            return;
        }
        std::string msg;
        std::string url;
        signal >> msg;
        signal >> url;
        progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, msg + " - " + url);
        multi_progress_bar.print();
    }
}


TransactionCB::TransactionCB(sdbus::IProxy * proxy, std::string session_object_path)
    : DbusCallback(proxy, session_object_path) {
    // register signal handlers
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_VERIFY_START, [this](sdbus::Signal & signal) -> void {
            this->verify_start(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_VERIFY_PROGRESS,
        [this](sdbus::Signal & signal) -> void { this->verify_progress(signal); });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_VERIFY_STOP, [this](sdbus::Signal & signal) -> void {
            this->verify_end(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_START,
        [this](sdbus::Signal & signal) -> void { this->transaction_start(signal); });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_PROGRESS,
        [this](sdbus::Signal & signal) -> void { this->transaction_progress(signal); });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_TRANSACTION_STOP,
        [this](sdbus::Signal & signal) -> void { this->transaction_end(signal); });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_ACTION_START, [this](sdbus::Signal & signal) -> void {
            this->action_start(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM,
        dnfdaemon::SIGNAL_TRANSACTION_ACTION_PROGRESS,
        [this](sdbus::Signal & signal) -> void { this->action_progress(signal); });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_ACTION_STOP, [this](sdbus::Signal & signal) -> void {
            this->action_end(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_START, [this](sdbus::Signal & signal) -> void {
            this->script_start(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_STOP, [this](sdbus::Signal & signal) -> void {
            this->script_stop(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_SCRIPT_ERROR, [this](sdbus::Signal & signal) -> void {
            this->script_error(signal);
        });
    proxy->registerSignalHandler(
        dnfdaemon::INTERFACE_RPM, dnfdaemon::SIGNAL_TRANSACTION_FINISHED, [this](sdbus::Signal & signal) -> void {
            this->finished(signal);
        });
}

void TransactionCB::new_progress_bar(uint64_t total, const std::string & description) {
    if (active_progress_bar && active_progress_bar->get_state() != libdnf::cli::progressbar::ProgressBarState::ERROR) {
        active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
    }
    auto progress_bar = std::make_unique<libdnf::cli::progressbar::DownloadProgressBar>(static_cast<int64_t>(total), description);
    multi_progress_bar.add_bar(progress_bar.get());
    progress_bar->set_auto_finish(false);
    progress_bar->start();
    active_progress_bar = progress_bar.get();
    progress_bars.push_back(std::move(progress_bar));
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
        int action_i;
        uint64_t total;
        signal >> nevra;
        signal >> action_i;
        signal >> total;
        auto action = static_cast<dnfdaemon::RpmTransactionItem::Actions>(action_i);
        std::string msg;
        switch (action) {
            case dnfdaemon::RpmTransactionItem::Actions::INSTALL:
                msg = "Installing";
                break;
            case dnfdaemon::RpmTransactionItem::Actions::ERASE:
                msg = "Erasing";
                break;
            case dnfdaemon::RpmTransactionItem::Actions::DOWNGRADE:
                msg = "Downgrading";
                break;
            case dnfdaemon::RpmTransactionItem::Actions::UPGRADE:
                msg = "Upgrading";
                break;
            case dnfdaemon::RpmTransactionItem::Actions::REINSTALL:
                msg = "Reinstalling";
                break;
            case dnfdaemon::RpmTransactionItem::Actions::CLEANUP:
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
        active_progress_bar->add_message(libdnf::cli::progressbar::MessageType::INFO, "Running scriptlet: " + nevra);
        multi_progress_bar.print();
    }
}

void TransactionCB::script_stop(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        std::string nevra;
        signal >> nevra;
        active_progress_bar->add_message(libdnf::cli::progressbar::MessageType::INFO, "Stop scriptlet: " + nevra);
        multi_progress_bar.print();
    }
}

void TransactionCB::script_error(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        std::string nevra;
        signal >> nevra;
        active_progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, "Error in scriptlet: " + nevra);
        multi_progress_bar.print();
    }
}

void TransactionCB::unpack_error(sdbus::Signal & signal) {
    if (signature_valid(signal) && active_progress_bar) {
        std::string nevra;
        signal >> nevra;
        active_progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, "Unpack error: " + nevra);
        multi_progress_bar.print();
    }
}

void TransactionCB::finished(sdbus::Signal & signal) {
    if (signature_valid(signal)) {
        if (active_progress_bar &&
            active_progress_bar->get_state() != libdnf::cli::progressbar::ProgressBarState::ERROR) {
            active_progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
        }
        multi_progress_bar.print();
    }
}
}  // namespace dnfdaemon::client
