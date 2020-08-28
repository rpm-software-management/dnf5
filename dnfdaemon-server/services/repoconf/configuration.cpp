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

#include "configuration.hpp"

#include <filesystem>
#include <glob.h>
#include <iostream>
#include <rpm/rpmlib.h>

Configuration::Configuration(std::string install_root) : Configuration()
{
    if (!install_root.empty()) {
        this->install_root = install_root;
    }
}

void Configuration::read_configuration()
{
    set_substitutions();
    read_main_config();
    read_repo_configs();
}

void Configuration::set_substitutions()
{
    //TODO use libdnf code to detect arch / basearch / releasever once it is ready
    substitutions["arch"] = "x86_64";
    substitutions["basearch"] = "x86_64";
    substitutions["releasever"] = "32";
}

std::string Configuration::prepend_install_root(std::string path)
{
    auto fs_path = std::filesystem::path(path);
    return (std::filesystem::path(install_root) / fs_path.relative_path()).string();
}

void Configuration::read_main_config()
{
    //TODO fix the logger: auto logger(libdnf::Log::getLogger());
    // create new main config parser and read the config file
    std::unique_ptr<libdnf::ConfigParser> main_parser(new libdnf::ConfigParser);
    main_parser->set_substitutions(substitutions);
    auto main_config_path = prepend_install_root(cfg_main->config_file_path().get_value());
    try {
        main_parser->read(main_config_path);
        const auto & cfgParserData = main_parser->get_data();
        // find the [main] section of the main config file
        const auto cfgParserDataIter = cfgParserData.find("main");
        if (cfgParserDataIter != cfgParserData.end()) {
            auto optBinds = cfg_main->opt_binds();
            const auto & cfgParserMainSect = cfgParserDataIter->second;
            for (const auto &opt : cfgParserMainSect) {
                auto option_key = opt.first;
                // find binding for the given key
                const auto optBindsIter = optBinds.find(option_key);
                if (optBindsIter != optBinds.end()) {
                    // set config option to value from the file with substituted vars
                    try {
                        optBindsIter->second.new_string(
                            libdnf::Option::Priority::MAINCONFIG,
                            main_parser->get_substituted_value("main", option_key));
                    } catch (const std::exception & e) {
                        /*XXX logger->warning(tfm::format("Config error in file \"%s\" section \"[main]\" key \"%s\": %s",
                                                    main_config_path, option_key, e.what()));*/
                    }
                }
            }
        }
        // read repos possibly configured in the main config file
        read_repos(main_parser.get(), main_config_path);
        // store the parser so it can be used for saving the config file later on
        config_parsers[std::move(main_config_path)] = std::move(main_parser);
    } catch (const std::exception & e) {
        //XXX logger->warning(tfm::format("Error parsing config file %s: %s", main_config_path, e.what()));
    }
}

void Configuration::read_repos(const libdnf::ConfigParser *repo_parser, const std::string &file_path)
{
    //XXX auto logger(libdnf::Log::getLogger());
    const auto & cfgParserData = repo_parser->get_data();
    for (const auto & cfgParserDataIter : cfgParserData) {
        if (cfgParserDataIter.first != "main") {
            // each section other than [main] is considered a repository
            auto section = cfgParserDataIter.first;
            const auto & sectionParser = cfgParserDataIter.second;
            std::unique_ptr<libdnf::rpm::ConfigRepo> cfgRepo(new libdnf::rpm::ConfigRepo(*cfg_main));
            auto optBinds = cfgRepo->opt_binds();
            for (const auto &opt : sectionParser) {
                auto option_key = opt.first;
                // skip the comment lines
                if (option_key[0] == '#') {
                    continue;
                }
                // find binding for the given key
                const auto optBindsIter = optBinds.find(option_key);
                if (optBindsIter != optBinds.end()) {
                    // configure the repo according the value from config file
                    try {
                        optBindsIter->second.new_string(
                            libdnf::Option::Priority::REPOCONFIG,
                            repo_parser->get_substituted_value(section, option_key));
                    } catch (const std::exception & e) {
                        /*XXX logger->warning(tfm::format("Config error in file \"%s\" section \"[%s]\" key \"%s\": %s",
                                                    file_path, section, option_key, e.what()));*/
                    }
                }
            }
            // save configured repo
            std::unique_ptr<RepoInfo> repoinfo(new RepoInfo());
            repoinfo->file_path = std::string(file_path);
            repoinfo->repoconfig = std::move(cfgRepo);
            repos[std::move(section)] = std::move(repoinfo);
        }
    }
}

void Configuration::read_repo_configs()
{
    //XXX auto logger(libdnf::Log::getLogger());
    for (auto reposDir : cfg_main->reposdir().get_value()) {
        // use canonical to resolve symlinks in reposDir
        std::string pattern;
        try {
            pattern = std::filesystem::canonical(prepend_install_root(reposDir)).string() + "/*.repo";
        }
        catch (std::filesystem::filesystem_error & e) {
            //XXX logger->debug(tfm::format("Error reading repository configuration directory %s: %s", reposDir, e.what()));
            continue;
        }
        glob_t globResult;
        glob(pattern.c_str(), GLOB_MARK, NULL, &globResult);
        for (size_t i = 0; i < globResult.gl_pathc; ++i) {
            std::unique_ptr<libdnf::ConfigParser> repo_parser(new libdnf::ConfigParser);
            std::string file_path = globResult.gl_pathv[i];
            repo_parser->set_substitutions(substitutions);
            try {
                repo_parser->read(file_path);
            } catch (libdnf::ConfigParser::Exception & e) {
                //XXX logger->warning(tfm::format("Error parsing config file %s: %s", file_path, e.what()));
                continue;
            }
            read_repos(repo_parser.get(), file_path);
            config_parsers[std::string(file_path)] = std::move(repo_parser);
        }
        globfree(&globResult);
    }
}

Configuration::RepoInfo* Configuration::find_repo(const std::string &repoid) {
    auto repo_iter = repos.find(repoid);
    if (repo_iter == repos.end()) {
        return NULL;
    }
    return repo_iter->second.get();
}

libdnf::ConfigParser* Configuration::find_parser(const std::string &file_path) {
    auto parser_iter = config_parsers.find(file_path);
    if (parser_iter == config_parsers.end()) {
        return NULL;
    }
    return parser_iter->second.get();
}
