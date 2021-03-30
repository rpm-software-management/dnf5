/*
Copyright (C) 2020-2021 Red Hat, Inc.

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

#include "repo_impl.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/conf/config_parser.hpp"
#include "libdnf/conf/option_bool.hpp"
#include "libdnf/rpm/solv_sack_impl.hpp"
#include "libdnf/utils/exception.hpp"

#include <fmt/format.h>

extern "C" {
#include <solv/repo.h>
#include <solv/solv_xfopen.h>
#include <solv/testcase.h>
}

#include <cerrno>
#include <filesystem>

using LibsolvRepo = ::Repo;


namespace libdnf::rpm {


RepoWeakPtr RepoSack::new_repo(const std::string & id) {
    // TODO(jrohel): Test repo exists
    auto repo = std::make_unique<Repo>(id, *base, Repo::Type::AVAILABLE);
    return add_item_with_return(std::move(repo));
}

// Deleter for std::unique_ptr<LibsolvRepo>
static void libsolv_repo_free(LibsolvRepo * libsolv_repo) {
    repo_free(libsolv_repo, 1);
}


RepoWeakPtr RepoSack::new_repo_from_libsolv_testcase(const std::string & repoid, const std::string & path) {
    std::unique_ptr<std::FILE, decltype(&std::fclose)> testcase_file(solv_xfopen(path.c_str(), "r"), &std::fclose);
    if (!testcase_file) {
        throw SystemError(EIO, "Unable to open libsolv testcase file", path);
    }

    auto repo = new_repo(repoid);
    Pool * pool = base->get_rpm_solv_sack().p_impl->get_pool();
    std::unique_ptr<LibsolvRepo, decltype(&libsolv_repo_free)> libsolv_repo(repo_create(pool, repoid.c_str()), &libsolv_repo_free);
    testcase_add_testtags(libsolv_repo.get(), testcase_file.get(), 0);
    repo->p_impl->attach_libsolv_repo(libsolv_repo.release());

    return repo;
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
        auto repo_id = base->get_vars().substitute(section);

        logger.debug(fmt::format(
            R"**(Start of loading configuration of repository "{}" from file "{}" section "{}")**",
            repo_id,
            path,
            section));

        auto bad_char_idx = Repo::verify_id(repo_id);
        if (bad_char_idx != std::string::npos) {
            auto msg = fmt::format(
                R"**(Bad id for repo "{}" section "{}", char = {} at pos {})**",
                repo_id,
                section,
                repo_id[bad_char_idx],
                bad_char_idx + 1);
            throw RuntimeError(msg);
        }

        auto repo = new_repo(repo_id);
        auto & repo_cfg = repo->get_config();
        repo_cfg.load_from_parser(parser, section, base->get_vars(), base->get_logger());
        logger.trace(fmt::format(R"**(Loading configuration of repository "{}" from file "{}" done)**", repo_id, path));

        if (repo_cfg.name().get_priority() == Option::Priority::DEFAULT) {
            logger.warning(fmt::format(
                "Repository \"{}\" is missing name in configuration file \"{}\", using id.", repo_id, path));
            repo_cfg.name().set(Option::Priority::REPOCONFIG, repo_id);
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
