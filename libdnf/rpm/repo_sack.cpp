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

#include "libdnf/rpm/repo_sack.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/conf/config_parser.hpp"
#include "libdnf/conf/option_bool.hpp"

#include <fmt/format.h>

#include <filesystem>

namespace libdnf::rpm {

RepoWeakPtr RepoSack::new_repo(const std::string & id) {
    // TODO(jrohel): Test repo exists
    auto repo_config = std::make_unique<ConfigRepo>(base->get_config());
    auto repo = std::make_unique<Repo>(id, std::move(repo_config), *base, Repo::Type::AVAILABLE);
    return add_item_with_return(std::move(repo));
}

static void load_config_from_parser(
    Config & conf, const ConfigParser & parser, const std::string & section, Base & base) {
    auto & logger = base.get_logger();
    const auto & cfg_parser_data = parser.get_data();
    auto cfg_parser_data_iter = cfg_parser_data.find(section);
    if (cfg_parser_data_iter != cfg_parser_data.end()) {
        auto opt_binds = conf.opt_binds();
        const auto & cfg_parser_sect = cfg_parser_data_iter->second;
        for (const auto & opt : cfg_parser_sect) {
            auto opt_binds_iter = opt_binds.find(opt.first);
            if (opt_binds_iter != opt_binds.end()) {
                try {
                    auto value = opt.second;
                    parser.substitute(value, base.get_variables());
                    opt_binds_iter->second.new_string(Option::Priority::REPOCONFIG, value);
                } catch (const Option::Exception & ex) {
                    auto msg = fmt::format(
                        R"**(Config error in section "{}" key "{}": {}: {})**",
                        section,
                        opt.first,
                        ex.get_description(),
                        ex.what());
                    logger.warning(msg);
                }
            }
        }
    }
}

void RepoSack::new_repos_from_file(const std::string & path) {
    auto & logger = base->get_logger();
    ConfigParser parser;
    parser.read(path);
    const auto & cfg_parser_data = parser.get_data();
    for (const auto & cfg_parser_data_iter : cfg_parser_data) {
        const auto & section = cfg_parser_data_iter.first;
        if (section == "main") {
            continue;
        }
        auto repo_id = section;
        parser.substitute(repo_id, base->get_variables());

        logger.debug(fmt::format(
            R"**(Start of loading configuration of repository "{}" from file "{}" section "{}")**",
            repo_id,
            path,
            section));

        auto bad_char_idx = Repo::verify_id(repo_id);
        if (bad_char_idx >= 0) {
            bool skip_if_unavailable;
            auto key_val = cfg_parser_data_iter.second.find("skip_if_unavailable");
            if (key_val != cfg_parser_data_iter.second.end()) {
                auto value = key_val->second;
                parser.substitute(value, base->get_variables());
                skip_if_unavailable = OptionBool(false).from_string(key_val->second);
            } else {
                skip_if_unavailable = base->get_config().skip_if_unavailable().get_value();
            }
            if (skip_if_unavailable) {
                auto msg = fmt::format(
                    R"**(Skipping repository with bad id "{}" section "{}", char = {} at pos {})**",
                    repo_id,
                    section,
                    repo_id[bad_char_idx],
                    bad_char_idx + 1);
                logger.warning(msg);
                continue;
            } else {
                auto msg = fmt::format(
                    R"**(Bad id for repo "{}" section "{}", char = {} at pos {})**",
                    repo_id,
                    section,
                    repo_id[bad_char_idx],
                    bad_char_idx + 1);
                throw RuntimeError(msg);
            }
        }

        auto repo = new_repo(repo_id);
        auto repo_cfg = repo->get_config();
        load_config_from_parser(*repo_cfg, parser, section, *base);
        logger.trace(fmt::format(R"**(Loading configuration of repository "{}" from file "{}" done)**", repo_id, path));

        if (repo_cfg->name().get_priority() == Option::Priority::DEFAULT) {
            logger.warning(fmt::format(
                "Repository \"{}\" is missing name in configuration file \"{}\", using id.", repo_id, path));
            repo_cfg->name().set(Option::Priority::REPOCONFIG, repo_id);
        }
    }
}

void RepoSack::new_repos_from_file() {
    new_repos_from_file(std::filesystem::path(base->get_config().config_file_path().get_value()));
}

void RepoSack::new_repos_from_dir(const std::string & dir_path) {
    std::filesystem::directory_iterator di(dir_path);
    std::vector<std::filesystem::path> paths;
    for (auto & dentry : di) {
        auto & path = dentry.path();
        if (dentry.is_regular_file() && path.extension() == ".repo") {
            paths.push_back(path);
        }
    }
    std::sort(paths.begin(), paths.end());
    for (auto & path : paths) {
        new_repos_from_file(path);
    }
}

void RepoSack::new_repos_from_dirs() {
    auto & logger = base->get_logger();
    for (auto & dir : base->get_config().reposdir().get_value()) {
        try {
            new_repos_from_dir(dir);
        } catch (const std::filesystem::filesystem_error & ex) {
            logger.info(ex.what());
        }
    }
}

}  // namespace libdnf::rpm
