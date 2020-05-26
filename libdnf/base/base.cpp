/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf/base/base.hpp"

#include "libdnf/conf/config_parser.hpp"
#include "libdnf/conf/const.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <filesystem>
#include <vector>

namespace libdnf {

static void load_config_from_parser(
    Config & conf, const ConfigParser & parser, const std::string & section, Logger & log) {
    const auto & cfg_parser_data = parser.get_data();
    auto cfg_parser_data_iter = cfg_parser_data.find(section);
    if (cfg_parser_data_iter != cfg_parser_data.end()) {
        auto opt_binds = conf.opt_binds();
        const auto & cfg_parser_main_sect = cfg_parser_data_iter->second;
        for (const auto & opt : cfg_parser_main_sect) {
            auto opt_binds_iter = opt_binds.find(opt.first);
            if (opt_binds_iter != opt_binds.end()) {
                try {
                    opt_binds_iter->second.new_string(libdnf::Option::Priority::MAINCONFIG, opt.second);
                } catch (const Option::Exception & ex) {
                    auto msg = fmt::format(
                        R"**(Config error in section "{}" key "{}": {}: {})**",
                        section,
                        opt.first,
                        ex.get_description(),
                        ex.what());
                    log.warning(msg);
                }
            }
        }
    }
}

static void load_config_from_file_path(
    Config & conf, const std::string & file, const std::string & section, Logger & log) {
    ConfigParser parser;
    parser.read(file);
    load_config_from_parser(conf, parser, section, log);
}

void Base::load_config_from_file(const std::string & path) {
    load_config_from_file_path(config, path, "main", log_router);
}

void Base::load_config_from_file() {
    load_config_from_file(config.config_file_path().get_value());
}

void Base::load_config_from_dir(const std::string & dir_path) {
    std::vector<std::filesystem::path> paths;
    for (auto & dentry : std::filesystem::directory_iterator(dir_path)) {
        auto & path = dentry.path();
        if (path.extension() == ".conf") {
            paths.push_back(path);
        }
    }
    std::sort(paths.begin(), paths.end());
    for (auto & path : paths) {
        load_config_from_file_path(config, path, "main", get_logger());
    }
}

void Base::load_config_from_dir() {
    load_config_from_dir(libdnf::CONF_DIRECTORY);
}

}  // namespace libdnf
