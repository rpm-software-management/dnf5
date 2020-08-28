/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "context.hpp"

#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf-cli/progressbar/multi_progress_bar.hpp>

#include <filesystem>
#include <iostream>
#include <string>

namespace microdnf {

/// Asks user for confirmaton
static bool userconfirm(libdnf::ConfigMain & config) {
    std::string msg;
    if (config.defaultyes().get_value()) {
        msg = "Is this ok [Y/n]: ";
    } else {
        msg = "Is this ok [y/N]: ";
    }
    while (true) {
        std::cout << msg;

        std::string choice;
        std::getline(std::cin, choice);

        if (choice.empty()) {
            return config.defaultyes().get_value();
        }
        if (choice == "y" || choice == "Y") {
            return true;
        }
        if (choice == "n" || choice == "N") {
            return false;
        }
    }
}

class MicrodnfRepoCB : public libdnf::rpm::RepoCB {
public:
    explicit MicrodnfRepoCB(libdnf::ConfigMain & config) : config(&config) {}

    void start(const char * what) override { std::cout << "Start downloading: \"" << what << "\"" << std::endl; }

    void end() override { std::cout << "Done." << std::endl; }

    // TODO(jrohel): Progress bar
    int progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) override {
        //std::cout << "Downloaded " << downloaded << "/" << total_to_download << std::endl;
        return 0;
    }

    bool repokey_import(
        const std::string & id,
        const std::string & user_id,
        const std::string & fingerprint,
        const std::string & url,
        [[maybe_unused]] long int timestamp) override {
        auto tmp_id = id.size() > 8 ? id.substr(id.size() - 8) : id;
        std::cout << "Importing GPG key 0x" << id << ":\n";
        std::cout << " Userid     : \"" << user_id << "\"\n";
        std::cout << " Fingerprint: " << fingerprint << "\n";
        std::cout << " From       : " << url << std::endl;

        if (config->assumeyes().get_value()) {
            return true;
        }
        if (config->assumeno().get_value()) {
            return false;
        }
        return userconfirm(*config);
    }

private:
    libdnf::ConfigMain * config;
};

void Context::load_rpm_repo(libdnf::rpm::Repo & repo) {
    //repo->set_substitutions(variables);
    auto & logger = base.get_logger();
    repo.set_callbacks(std::make_unique<microdnf::MicrodnfRepoCB>(base.get_config()));
    try {
        repo.load();
    } catch (const std::runtime_error & ex) {
        logger.warning(ex.what());
        std::cout << ex.what() << std::endl;
    }
}

class PkgDownloadCB : public libdnf::rpm::PackageTargetCB {
public:
    PkgDownloadCB(libdnf::cli::progressbar::MultiProgressBar & mp_bar, const std::string & what)
        : multi_progress_bar(&mp_bar), what(what) {
            progress_bar = new libdnf::cli::progressbar::DownloadProgressBar(-1, what);
            multi_progress_bar->add_bar(progress_bar);
        }

    int end(TransferStatus status, const char * msg) override {
        switch (status) {
            case TransferStatus::SUCCESSFUL:
                //std::cout << "[DONE] " << what << std::endl;
                break;
            case TransferStatus::ALREADYEXISTS:
                //std::cout << "[SKIPPED] " << what << ": " << msg << std::endl;
                progress_bar->add_message(libdnf::cli::progressbar::MessageType::SUCCESS, msg);
                progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
                break;
            case TransferStatus::ERROR:
                //std::cout << "[ERROR] " << what << ": " << msg << std::endl;
                progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, msg);
                progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
                break;
        }
        multi_progress_bar->print();
        return 0;
    }

    int progress(double total_to_download, double downloaded) override {
        auto total = static_cast<int64_t>(total_to_download);
        if (total > 0) {
            progress_bar->set_total_ticks(total);
        }
        if (progress_bar->get_state() == libdnf::cli::progressbar::ProgressBarState::READY) {
            progress_bar->start();
        }
        progress_bar->set_ticks(static_cast<int64_t>(downloaded));
        if (is_time_to_print()) {
            multi_progress_bar->print();
        }
        return 0;
    }

    int mirror_failure(const char * msg, const char * url) override {
        //std::cout << "Mirror failure: " << msg << " " << url << std::endl;
        std::string message = std::string(msg) + " - " + url;
        progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, message);
        return 0;
    }

private:
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

    static std::chrono::time_point<std::chrono::steady_clock> prev_print_time;

    libdnf::cli::progressbar::MultiProgressBar * multi_progress_bar;
    libdnf::cli::progressbar::DownloadProgressBar * progress_bar;
    std::string what;
};

std::chrono::time_point<std::chrono::steady_clock> PkgDownloadCB::prev_print_time = std::chrono::steady_clock::now();

void download_packages(libdnf::rpm::PackageSet & package_set, const char * dest_dir) {
    libdnf::cli::progressbar::MultiProgressBar multi_progress_bar;
    std::vector<std::unique_ptr<PkgDownloadCB>> pkg_download_callbacks_guard;
    std::vector<std::unique_ptr<libdnf::rpm::PackageTarget>> targets_guard;
    std::vector<libdnf::rpm::PackageTarget *> targets;

    std::string destination;
    if (dest_dir) {
        destination = dest_dir;
    }
    for (auto package : package_set) {
        auto repo = package.get_repo();
        auto checksum = package.get_checksum();
        if (!dest_dir) {
            destination = std::filesystem::path(repo->get_cachedir()) / "packages";
            std::filesystem::create_directory(destination);
        }

        auto pkg_download_cb = std::make_unique<PkgDownloadCB>(multi_progress_bar, package.get_nevra());
        auto pkg_download_cb_ptr = pkg_download_cb.get();
        pkg_download_callbacks_guard.push_back(std::move(pkg_download_cb));

        auto pkg_target = std::make_unique<libdnf::rpm::PackageTarget>(
            repo,
            package.get_location().c_str(),
            destination.c_str(),
            static_cast<int>(checksum.get_type()),
            checksum.get_checksum().c_str(),
            static_cast<int64_t>(package.get_download_size()),
            package.get_baseurl().empty() ? nullptr : package.get_baseurl().c_str(),
            true,
            0,
            0,
            pkg_download_cb_ptr);
        targets.push_back(pkg_target.get());
        targets_guard.push_back(std::move(pkg_target));
    }

    std::cout << "Downloading Packages:" << std::endl;
    try {
        libdnf::rpm::PackageTarget::download_packages(targets, true);
    } catch (const std::runtime_error & ex) {
        std::cout << "Exception: " << ex.what() << std::endl;
    }
}

}  // namespace microdnf
