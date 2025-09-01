// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "configuration.hpp"

#include <glob.h>
#include <rpm/rpmlib.h>

#include <filesystem>
#include <iostream>

Configuration::Configuration(Session & session) : session(session) {}

void Configuration::read_configuration() {
    read_main_config();
    read_repo_configs();
}

void Configuration::read_main_config() {
    auto & logger = *session.get_base()->get_logger();
    auto base = session.get_base();
    auto & cfg_main = base->get_config();
    auto main_config_path = cfg_main.get_config_file_path_option().get_value();

    try {
        // create new main config parser and read the config file
        std::unique_ptr<libdnf5::ConfigParser> main_parser(new libdnf5::ConfigParser);

        main_parser->read(main_config_path);
        cfg_main.load_from_parser(*main_parser, "main", *base->get_vars(), *base->get_logger());

        // read repos possibly configured in the main config file
        read_repos(main_parser.get(), main_config_path);
        // store the parser so it can be used for saving the config file later on
        config_parsers[std::move(main_config_path)] = std::move(main_parser);
    } catch (const std::exception & e) {
        logger.warning("Error parsing config file \"{}\": {}", main_config_path, e.what());
    }
}

void Configuration::read_repos(const libdnf5::ConfigParser * repo_parser, const std::string & file_path) {
    const auto & cfg_parser_data = repo_parser->get_data();
    auto base = session.get_base();
    auto & cfg_main = base->get_config();
    for (const auto & cfg_parser_data_iter : cfg_parser_data) {
        if (cfg_parser_data_iter.first != "main") {
            // each section other than [main] is considered a repository
            auto section = cfg_parser_data_iter.first;
            // TODO(lukash) normally the repo id is the section with vars substituted, shouldn't be different here
            std::unique_ptr<libdnf5::repo::ConfigRepo> cfg_repo(new libdnf5::repo::ConfigRepo(cfg_main, section));

            cfg_repo->load_from_parser(*repo_parser, section, *base->get_vars(), *base->get_logger());

            // save configured repo
            std::unique_ptr<RepoInfo> repoinfo(new RepoInfo());
            repoinfo->file_path = std::string(file_path);
            repoinfo->repoconfig = std::move(cfg_repo);
            repos[std::move(section)] = std::move(repoinfo);
        }
    }
}

void Configuration::read_repo_configs() {
    auto base = session.get_base();
    auto & logger = *base->get_logger();
    auto & cfg_main = base->get_config();
    for (const auto & repos_dir : cfg_main.get_reposdir_option().get_value()) {
        // use canonical to resolve symlinks in repos_dir
        std::string pattern;
        try {
            pattern = std::filesystem::canonical(repos_dir).string() + "/*.repo";
        } catch (std::filesystem::filesystem_error & e) {
            logger.debug("Error reading repository configuration directory \"{}\": {}", repos_dir, e.what());
            continue;
        }
        glob_t glob_result;
        glob(pattern.c_str(), GLOB_MARK, nullptr, &glob_result);
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            std::unique_ptr<libdnf5::ConfigParser> repo_parser(new libdnf5::ConfigParser);
            std::string file_path = glob_result.gl_pathv[i];
            try {
                repo_parser->read(file_path);
            } catch (libdnf5::ConfigParserError & e) {
                logger.warning("Error parsing config file \"{}\": {}", file_path, e.what());
                continue;
            }
            read_repos(repo_parser.get(), file_path);
            config_parsers[std::string(file_path)] = std::move(repo_parser);
        }
        globfree(&glob_result);
    }
}

Configuration::RepoInfo * Configuration::find_repo(const std::string & repoid) {
    auto repo_iter = repos.find(repoid);
    if (repo_iter == repos.end()) {
        return nullptr;
    }
    return repo_iter->second.get();
}

libdnf5::ConfigParser * Configuration::find_parser(const std::string & file_path) {
    auto parser_iter = config_parsers.find(file_path);
    if (parser_iter == config_parsers.end()) {
        return nullptr;
    }
    return parser_iter->second.get();
}
