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

#include "utils.hpp"

#include <libdnf/base/base.hpp>
#include <libdnf/logger/memory_buffer_logger.hpp>
#include <libdnf/logger/stream_logger.hpp>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

int main() {
    libdnf::Base base;

    auto & log_router = base.get_logger();

    // Add circular memory buffer logger
    const std::size_t max_log_items_to_keep = 10000;
    const std::size_t prealloc_log_items = 256;
    log_router.add_logger(std::make_unique<libdnf::MemoryBufferLogger>(max_log_items_to_keep, prealloc_log_items));

    log_router.info("Microdnf start");

    base.load_config_from_file();

    // Without "root" effective privileges program switches to user specific directories
    if (!microdnf::am_i_root()) {
        auto tmp = fs::temp_directory_path() / "microdnf";
        if (base.get_config().logdir().get_priority() < libdnf::Option::Priority::COMMANDLINE) {
            auto logdir = tmp / "log";
            base.get_config().logdir().set(libdnf::Option::Priority::RUNTIME, logdir);
            fs::create_directories(logdir);
        }
        if (base.get_config().cachedir().get_priority() < libdnf::Option::Priority::COMMANDLINE) {
            // Sets path to cache directory.
            auto cache_dir = microdnf::xdg::get_user_cache_dir() / "microdnf";
            base.get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, cache_dir);
        }
    }

    // Swap to destination logger (log to file) and write messages from memory buffer logger to it
    auto log_file = fs::path(base.get_config().logdir().get_value()) / "microdnf.log";
    auto log_stream = std::make_unique<std::ofstream>(log_file, std::ios::app);
    std::unique_ptr<libdnf::Logger> logger = std::make_unique<libdnf::StreamLogger>(std::move(log_stream));
    log_router.swap_logger(logger, 0);
    dynamic_cast<libdnf::MemoryBufferLogger &>(*logger).write_to_logger(log_router);

    // detect values of basic variables (arch, basearch, and releasever) for substitutions
    auto arch = microdnf::detect_arch();
    auto & variables = base.get_variables();
    variables["arch"] = arch;
    variables["basearch"] = microdnf::get_base_arch(arch.c_str());
    variables["releasever"] = microdnf::detect_release(base.get_config().installroot().get_value());

    // load additional variables from environment and directories
    libdnf::ConfigMain::add_vars_from_env(variables);
    for (auto & dir : base.get_config().varsdir().get_value()) {
        libdnf::ConfigMain::add_vars_from_dir(variables, dir);
    }

    // create rpm repositories according configuration files
    auto & rpm_repo_sack = base.get_rpm_repo_sack();
    rpm_repo_sack.new_repos_from_file();
    rpm_repo_sack.new_repos_from_dirs();

    log_router.info("Microdnf end");

    return 0;
}
