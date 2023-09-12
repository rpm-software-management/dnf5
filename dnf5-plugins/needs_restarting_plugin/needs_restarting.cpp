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

#include "needs_restarting.hpp"

#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5-cli/output/changelogs.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace dnf5 {

using namespace libdnf5::cli;

void NeedsRestartingCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void NeedsRestartingCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Determine whether system or systemd services need restarting");
}

void NeedsRestartingCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);

    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);

    context.base.get_config().get_optional_metadata_types_option().add_item(
        libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_UPDATEINFO);
}

time_t get_boot_time() {
    time_t proc_1_boot_time = 0;
    struct stat proc_1_stat = {};
    if (stat("/proc/1", &proc_1_stat) == 0) {
        proc_1_boot_time = proc_1_stat.st_mtime;
    }

    time_t uptime_boot_time = 0;
    std::ifstream uptime_stream{"/proc/uptime"};
    if (uptime_stream.is_open()) {
        double uptime = 0;
        uptime_stream >> uptime;
        if (uptime > 0) {
            uptime_boot_time = std::time(nullptr) - static_cast<time_t>(uptime);
        }
    }

    return std::max(proc_1_boot_time, uptime_boot_time);
}

void NeedsRestartingCommand::run() {
    auto & ctx = get_context();

    const auto boot_time = get_boot_time();

    libdnf5::rpm::PackageQuery base_query{ctx.base};

    libdnf5::rpm::PackageQuery installed{base_query};
    installed.filter_installed();

    libdnf5::rpm::PackageQuery reboot_suggested{installed};
    reboot_suggested.filter_reboot_suggested();

    std::vector<libdnf5::rpm::Package> need_reboot = {};
    for (const auto & pkg : reboot_suggested) {
        if (pkg.get_install_time() > static_cast<unsigned long long>(boot_time)) {
            need_reboot.push_back(pkg);
        }
    }

    if (need_reboot.empty()) {
        std::cout << "No core libraries or services have been updated since boot-up." << std::endl
                  << "Reboot should not be necessary." << std::endl;
    } else {
        std::cout << "Core libraries or services have been updated since boot-up:" << std::endl;
        for (const auto & pkg : need_reboot) {
            std::cout << "\t" << pkg.get_name() << std::endl;
        }
        throw libdnf5::cli::SilentCommandExitError(1);
    }
}

}  // namespace dnf5
