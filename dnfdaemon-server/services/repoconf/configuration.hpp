/*
 * Copyright (C) 2020 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef DNFDAEMON_SERVER_SERVICES_REPOCONF_CONFIGURATION_HPP
#define DNFDAEMON_SERVER_SERVICES_REPOCONF_CONFIGURATION_HPP

#include "libdnf/conf/config_main.hpp"
#include "libdnf/conf/config_parser.hpp"
#include "libdnf/rpm/config_repo.hpp"

#include <map>
#include <memory>
#include <vector>


class Configuration {
public:
    struct RepoInfo {
        std::string file_path;
        std::unique_ptr<libdnf::rpm::ConfigRepo> repoconfig;
    };

    Configuration() : cfg_main(new libdnf::ConfigMain), install_root("/"){};
    explicit Configuration(const std::string & install_root);
    ~Configuration() = default;

    void read_configuration();
    const std::map<std::string, std::unique_ptr<RepoInfo>> & get_repos() { return repos; }
    RepoInfo * find_repo(const std::string & repoid);
    libdnf::ConfigParser * find_parser(const std::string & file_path);

private:
    std::unique_ptr<libdnf::ConfigMain> cfg_main;
    // repoid: repoinfo
    std::map<std::string, std::unique_ptr<RepoInfo>> repos;
    // repo_config_file_path: parser
    std::map<std::string, std::unique_ptr<libdnf::ConfigParser>> config_parsers;
    std::map<std::string, std::string> substitutions;
    std::string install_root;

    void read_repos(const libdnf::ConfigParser * repo_parser, const std::string & file_path);
    void set_substitutions();
    void read_main_config();
    void read_repo_configs();
    std::string prepend_install_root(const std::string & path);
};


#endif
