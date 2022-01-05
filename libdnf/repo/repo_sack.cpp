/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf/repo/repo_sack.hpp"

#include "repo_impl.hpp"
#include "rpm/package_sack_impl.hpp"
#include "utils/bgettext/bgettext-lib.h"

#include "libdnf/base/base.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/conf/config_parser.hpp"
#include "libdnf/conf/option_bool.hpp"

#include <fmt/format.h>

extern "C" {
#include <solv/repo.h>
#include <solv/solv_xfopen.h>
#include <solv/testcase.h>
}

#include <atomic>
#include <cerrno>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>


using LibsolvRepo = ::Repo;

namespace {

// Names of special repositories
constexpr const char * SYSTEM_REPO_NAME = "@System";
constexpr const char * CMDLINE_REPO_NAME = "@commandline";
// TODO lukash: unused, remove?
//constexpr const char * MODULE_FAIL_SAFE_REPO_NAME = "@modulefailsafe";

}  // namespace

namespace libdnf::repo {

RepoSack::RepoSack(libdnf::Base & base) : RepoSack(base.get_weak_ptr()) {}


RepoWeakPtr RepoSack::new_repo(const std::string & id) {
    // TODO(jrohel): Test repo exists
    auto repo = std::make_unique<Repo>(base, id, Repo::Type::AVAILABLE);
    return add_item_with_return(std::move(repo));
}


RepoWeakPtr RepoSack::new_repo_from_libsolv_testcase(const std::string & repoid, const std::string & path) {
    std::unique_ptr<std::FILE, decltype(&std::fclose)> testcase_file(solv_xfopen(path.c_str(), "r"), &std::fclose);
    if (!testcase_file) {
        throw SystemError(errno ? errno : EIO, M_("Unable to open libsolv testcase file \"{}\""), path);
    }

    auto repo = new_repo(repoid);
    testcase_add_testtags(repo->p_impl->solv_repo.repo, testcase_file.get(), 0);
    return repo;
}


RepoWeakPtr RepoSack::get_cmdline_repo() {
    if (!cmdline_repo) {
        std::unique_ptr<Repo> repo(new Repo(base, CMDLINE_REPO_NAME, Repo::Type::COMMANDLINE));
        cmdline_repo = repo.get();
        add_item(std::move(repo));
    }

    return cmdline_repo->get_weak_ptr();
}


RepoWeakPtr RepoSack::get_system_repo() {
    if (!system_repo) {
        std::unique_ptr<Repo> repo(new Repo(base, SYSTEM_REPO_NAME, Repo::Type::SYSTEM));
        system_repo = repo.get();
        add_item(std::move(repo));
        pool_set_installed(*get_pool(base), system_repo->p_impl->solv_repo.repo);
    }

    return system_repo->get_weak_ptr();
}


void RepoSack::update_and_load_repos(libdnf::repo::RepoQuery & repos, Repo::LoadFlags flags) {
    std::atomic<bool> except{false};  // set to true if an exception occurred
    std::exception_ptr except_ptr;    // for pass exception from thread_sack_loader to main thread,
                                      // a default-constructed std::exception_ptr is a null pointer

    std::vector<libdnf::repo::Repo *> prepared_repos;  // array of repositories prepared to load into solv sack
    std::mutex prepared_repos_mutex;                   // mutex for the array
    std::condition_variable signal_prepared_repo;      // signals that next item is added into array
    std::size_t num_repos_loaded{0};                   // number of repositories already loaded into solv sack

    prepared_repos.reserve(repos.size() + 1);  // optimization: preallocate memory to avoid realocations, +1 stop tag

    // This thread loads prepared repositories into solvable sack
    std::thread thread_sack_loader([&]() {
        try {
            while (true) {
                std::unique_lock<std::mutex> lock(prepared_repos_mutex);
                while (prepared_repos.size() <= num_repos_loaded) {
                    signal_prepared_repo.wait(lock);
                }
                auto repo = prepared_repos[num_repos_loaded];
                lock.unlock();

                if (!repo || except) {
                    break;  // nullptr mark - work is done, or exception in main thread
                }

                repo->load(flags);
                ++num_repos_loaded;
            }
        } catch (std::runtime_error & ex) {
            // The thread must not throw exceptions. Pass them to the main thread using exception_ptr.
            except_ptr = std::current_exception();
            except = true;
        }
    });

    // Adds information that all repos are updated (nullptr tag) and is waiting for thread_sack_loader to complete.
    auto finish_sack_loader = [&]() {
        {
            std::lock_guard<std::mutex> lock(prepared_repos_mutex);
            prepared_repos.push_back(nullptr);
        }
        signal_prepared_repo.notify_one();

        thread_sack_loader.join();  // waits for the thread_sack_loader to finish its execution
    };

    auto catch_thread_sack_loader_exceptions = [&]() {
        if (except) {
            if (thread_sack_loader.joinable()) {
                thread_sack_loader.join();
            }

            std::rethrow_exception(except_ptr);
        }
    };

    // If the input RepoQuery contains a system repo, we load the system repo first.
    for (auto & repo : repos) {
        if (repo->get_type() == libdnf::repo::Repo::Type::SYSTEM) {
            {
                std::lock_guard<std::mutex> lock(prepared_repos_mutex);
                prepared_repos.push_back(repo.get());
            }

            signal_prepared_repo.notify_one();
            break;
        }
    }

    // Prepares available repositories metadata for thread sack loader.
    for (auto & repo : repos) {
        if (repo->get_type() != libdnf::repo::Repo::Type::AVAILABLE) {
            continue;
        }
        catch_thread_sack_loader_exceptions();
        try {
            repo->fetch_metadata();

            {
                std::lock_guard<std::mutex> lock(prepared_repos_mutex);
                prepared_repos.push_back(repo.get());
            }

            signal_prepared_repo.notify_one();
        } catch (const libdnf::repo::RepoDownloadError & e) {
            if (!repo->get_config().skip_if_unavailable().get_value()) {
                except = true;
                finish_sack_loader();
                throw;
            } else {
                base->get_logger()->warning(utils::sformat(
                    "Error loading repository \"{}\" (skipping due to \"skip_if_unavailable=true\"): {}",
                    repo->get_id(),
                    e.what()));  // TODO(lukash) we should print nested exceptions
            }
        } catch (const std::runtime_error & e) {
            except = true;
            finish_sack_loader();
            throw;
        }
    }

    finish_sack_loader();
    catch_thread_sack_loader_exceptions();

    base->get_rpm_package_sack()->setup_excludes_includes();
}


void RepoSack::update_and_load_enabled_repos(bool load_system, Repo::LoadFlags flags) {
    if (load_system) {
        // create the system repository if it does not exist
        base->get_repo_sack()->get_system_repo();
    }

    libdnf::repo::RepoQuery repos(base);
    repos.filter_enabled(true);

    if (!load_system) {
        repos.filter_type(Repo::Type::SYSTEM, libdnf::sack::QueryCmp::NEQ);
    }

    update_and_load_repos(repos, flags);
}


void RepoSack::dump_debugdata(const std::string & dir) {
    Solver * solver = solver_create(*get_pool(base));

    try {
        std::filesystem::create_directory(dir);

        int ret = testcase_write(solver, dir.c_str(), 0, NULL, NULL);
        if (!ret) {
            throw SystemError(errno, fmt::format("Failed to write debug data to {}", dir));
        }
    } catch (...) {
        solver_free(solver);
        throw;
    }
    solver_free(solver);
}


void RepoSack::new_repos_from_file(const std::string & path) {
    auto & logger = *base->get_logger();
    ConfigParser parser;
    parser.read(path);
    const auto & cfg_parser_data = parser.get_data();
    for (const auto & cfg_parser_data_iter : cfg_parser_data) {
        const auto & section = cfg_parser_data_iter.first;
        if (section == "main") {
            continue;
        }
        auto repo_id = base->get_vars()->substitute(section);

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
        repo_cfg.load_from_parser(parser, section, *base->get_vars(), *base->get_logger());
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
    auto & logger = *base->get_logger();
    for (auto & dir : base->get_config().reposdir().get_value()) {
        try {
            new_repos_from_dir(dir);
        } catch (const std::filesystem::filesystem_error & ex) {
            logger.info(ex.what());
        }
    }
}

BaseWeakPtr RepoSack::get_base() const {
    return base;
}


void RepoSack::internalize_repos() {
    auto rq = RepoQuery(base);
    for (auto & repo : rq.get_data()) {
        repo->p_impl->solv_repo.internalize();
    }

    if (system_repo) {
        system_repo->p_impl->solv_repo.internalize();
    }

    if (cmdline_repo) {
        cmdline_repo->p_impl->solv_repo.internalize();
    }
}

}  // namespace libdnf::repo
