// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5DAEMON_SERVER_SERVICES_REPO_CONFIGURATION_HPP
#define DNF5DAEMON_SERVER_SERVICES_REPO_CONFIGURATION_HPP

#include "session.hpp"

#include <libdnf5/conf/config_main.hpp>
#include <libdnf5/conf/config_parser.hpp>
#include <libdnf5/repo/config_repo.hpp>

#include <map>
#include <memory>
#include <vector>


class Configuration {
public:
    struct RepoInfo {
        std::string file_path;
        std::unique_ptr<libdnf5::repo::ConfigRepo> repoconfig;
    };

    explicit Configuration(Session & session);
    ~Configuration() = default;

    void read_configuration();
    const std::map<std::string, std::unique_ptr<RepoInfo>> & get_repos() { return repos; }
    RepoInfo * find_repo(const std::string & repoid);
    libdnf5::ConfigParser * find_parser(const std::string & file_path);

private:
    // repoid: repoinfo
    std::map<std::string, std::unique_ptr<RepoInfo>> repos;
    // repo_config_file_path: parser
    std::map<std::string, std::unique_ptr<libdnf5::ConfigParser>> config_parsers;
    std::map<std::string, std::string> substitutions;
    Session & session;

    void read_repos(const libdnf5::ConfigParser * repo_parser, const std::string & file_path);
    void read_main_config();
    void read_repo_configs();
};


#endif
