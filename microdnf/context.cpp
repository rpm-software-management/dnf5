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

    void end() override { std::cout << "End downloading" << std::endl; }

    // TODO(jrohel): Progress bar
    int progress([[maybe_unused]] double totalToDownload, [[maybe_unused]] double downloaded) override {
        //std::cout << "Downloaded " << downloaded << "/" << totalToDownload << std::endl;
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

}  // namespace microdnf
