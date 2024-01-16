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

#include "libdnf5/repo/repo_sack.hpp"

#include "../module/module_sack_impl.hpp"
#include "conf/config.h"
#include "repo_cache_private.hpp"
#include "repo_downloader.hpp"
#include "rpm/package_sack_impl.hpp"
#include "solv/solver.hpp"
#include "solv_repo.hpp"
#include "utils/auth.hpp"
#include "utils/fs/utils.hpp"
#include "utils/string.hpp"
#include "utils/url.hpp"
#include "utils/xml.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/comps/environment/query.hpp"
#include "libdnf5/comps/group/query.hpp"
#include "libdnf5/conf/config_parser.hpp"
#include "libdnf5/conf/const.hpp"
#include "libdnf5/conf/option_bool.hpp"
#include "libdnf5/repo/file_downloader.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/file.hpp"
#include "libdnf5/utils/fs/temp.hpp"

extern "C" {
#include <solv/testcase.h>
}

#include <atomic>
#include <cerrno>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <sstream>
#include <thread>


using LibsolvRepo = ::Repo;

namespace fs = std::filesystem;

namespace {

// Names of special repositories
constexpr const char * SYSTEM_REPO_NAME = "@System";
constexpr const char * CMDLINE_REPO_NAME = "@commandline";
// TODO lukash: unused, remove?
//constexpr const char * MODULE_FAIL_SAFE_REPO_NAME = "@modulefailsafe";

}  // namespace

namespace libdnf5::repo {

RepoSack::RepoSack(libdnf5::Base & base) : RepoSack(base.get_weak_ptr()) {}


RepoWeakPtr RepoSack::create_repo(const std::string & id) {
    for (const auto & existing_repo : get_data()) {
        if (existing_repo->get_id() == id) {
            throw RepoIdAlreadyExistsError(
                M_("Failed to create repo \"{}\": Id is present more than once in the configuration"), id);
        }
    }
    auto repo = std::make_unique<Repo>(base, id, Repo::Type::AVAILABLE);
    return add_item_with_return(std::move(repo));
}


RepoWeakPtr RepoSack::create_repo_from_libsolv_testcase(const std::string & id, const std::string & path) {
    auto repo = create_repo(id);
    repo->add_libsolv_testcase(path);
    return repo;
}


RepoWeakPtr RepoSack::get_cmdline_repo() {
    if (!cmdline_repo) {
        std::unique_ptr<Repo> repo(new Repo(base, CMDLINE_REPO_NAME, Repo::Type::COMMANDLINE));
        repo->get_config().get_build_cache_option().set(libdnf5::Option::Priority::RUNTIME, false);
        cmdline_repo = repo.get();
        add_item(std::move(repo));
    }

    return cmdline_repo->get_weak_ptr();
}


std::map<std::string, libdnf5::rpm::Package> RepoSack::add_cmdline_packages(
    const std::vector<std::string> & paths, bool calculate_checksum) {
    // find remote URLs and local file paths in the input
    std::vector<std::string> rpm_urls;
    std::vector<std::string> rpm_filepaths;
    const std::string_view ext(".rpm");
    for (const auto & spec : paths) {
        if (libdnf5::utils::url::is_url(spec)) {
            rpm_urls.emplace_back(spec);
        } else if (spec.length() > ext.length() && spec.ends_with(ext)) {
            rpm_filepaths.emplace_back(spec);
        }
    }

    if (rpm_urls.empty() && rpm_filepaths.empty()) {
        // nothing to fill into cmdline repo
        return {};
    }

    auto cmdline_repo = get_cmdline_repo();

    // Ensure that download location for command line repository exists
    std::filesystem::path cmd_repo_pkgs_dir{cmdline_repo->get_cachedir()};
    cmd_repo_pkgs_dir /= CACHE_PACKAGES_DIR;
    std::filesystem::create_directories(cmd_repo_pkgs_dir);

    // map remote URLs to local destination files
    std::map<std::string_view, std::filesystem::path> url_to_path;
    for (const auto & url : rpm_urls) {
        if (url_to_path.contains(url)) {
            // only unique URLs go to downloader
            continue;
        }
        std::filesystem::path path{url.substr(url.find("://") + 3)};
        // To deal with URLs that do not contain a filename
        // (e.g. http://location/package/) prepend destination path with the
        // hash of the URL.
        // It also solves a corner case when multiple URLs share
        // the same "filename" - e.g. http://location1/package.rpm and
        // http://location2/package.rpm should be treated as two different
        // packages.
        // It's similar to constructing the cache dir for repository metadata.
        std::stringstream sstream;
        sstream << std::hex << std::hash<std::string>{}(url) << "-" << path.filename().string();
        auto dest_path = sstream.str();
        if (!dest_path.ends_with(ext)) {
            // Add ".rpm" extension to those URLs that do not have it.
            // (e.g. http://location/package?name=test)
            dest_path += ext;
        }
        url_to_path.emplace(url, cmd_repo_pkgs_dir / dest_path);
    }

    // map a path from the input paths to a Package object created in the cmdline repo
    std::map<std::string, libdnf5::rpm::Package> path_to_package;

    if (!url_to_path.empty()) {
        auto & logger = *base->get_logger();
        // download remote URLs
        libdnf5::repo::FileDownloader downloader(base);
        for (auto & [url, dest_path] : url_to_path) {
            logger.debug("Downloading package \"{}\" to file \"{}\"", url, dest_path.string());
            // TODO(mblaha): temporarily used the dummy DownloadCallbacks instance
            downloader.add(std::string{url}, dest_path.string());
        }
        downloader.download();

        // fill the command line repo with downloaded URLs
        for (const auto & [url, path] : url_to_path) {
            path_to_package.emplace(url, cmdline_repo->add_rpm_package(path.string(), calculate_checksum));
        }
    }

    // fill the command line repo with local files
    for (const auto & path : rpm_filepaths) {
        if (!path_to_package.contains(path)) {
            path_to_package.emplace(path, cmdline_repo->add_rpm_package(path, calculate_checksum));
        }
    }

    if (!path_to_package.empty()) {
        base->get_rpm_package_sack()->load_config_excludes_includes();
    }

    return path_to_package;
}


RepoWeakPtr RepoSack::get_system_repo() {
    if (!system_repo) {
        std::unique_ptr<Repo> repo(new Repo(base, SYSTEM_REPO_NAME, Repo::Type::SYSTEM));
        // TODO(mblaha): re-enable caching once we can reliably detect whether system repo is up-to-date
        repo->get_config().get_build_cache_option().set(libdnf5::Option::Priority::RUNTIME, false);
        system_repo = repo.get();
        add_item(std::move(repo));
    }

    return system_repo->get_weak_ptr();
}

/**
 *
 * @param repos Set of repositories to load
 * @param import_keys Whether or not to import signing keys
 * @warning This function should not be used to load and update repositories. Instead, use `RepoSack::update_and_load_enabled_repos`
 */
void RepoSack::update_and_load_repos(libdnf5::repo::RepoQuery & repos, bool import_keys) {
    auto logger = base->get_logger();

    std::atomic<bool> except_in_main_thread{false};  // set to true if an exception occurred in the main thread
    std::exception_ptr except_ptr;                   // for pass exception from thread_sack_loader to main thread,
                                                     // a default-constructed std::exception_ptr is a null pointer

    std::vector<Repo *> prepared_repos;            // array of repositories prepared to load into solv sack
    std::mutex prepared_repos_mutex;               // mutex for the array
    std::condition_variable signal_prepared_repo;  // signals that next item is added into array
    std::size_t num_repos_loaded{0};               // number of repositories already loaded into solv sack

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

                if (!repo || except_in_main_thread) {
                    break;  // nullptr mark - work is done, or exception in main thread
                }

                repo->load();
                ++num_repos_loaded;
            }
        } catch (std::runtime_error & ex) {
            // The thread must not throw exceptions. Pass them to the main thread using exception_ptr.
            except_ptr = std::current_exception();
        }
    });

    // Add repository to array of repositories prepared to load into solv sack.
    auto send_to_sack_loader = [&](repo::Repo * repo) {
        {
            std::lock_guard<std::mutex> lock(prepared_repos_mutex);
            prepared_repos.push_back(repo);
        }
        signal_prepared_repo.notify_one();
    };

    // Adds information that all repos are updated (nullptr tag) and is waiting for thread_sack_loader to complete.
    auto finish_sack_loader = [&]() {
        send_to_sack_loader(nullptr);
        thread_sack_loader.join();  // waits for the thread_sack_loader to finish its execution
    };

    auto catch_thread_sack_loader_exceptions = [&]() {
        if (except_ptr) {
            if (thread_sack_loader.joinable()) {
                thread_sack_loader.join();
            }

            std::rethrow_exception(except_ptr);
        }
    };

    auto handle_repo_download_error = [&](const Repo * repo, const RepoDownloadError & e, bool report_key_err) {
        if (report_key_err) {
            try {
                std::rethrow_if_nested(e);
            } catch (const LibrepoError & lr_err) {
                if (lr_err.get_code() == LRE_BADGPG) {
                    return true;
                }
            } catch (...) {
            }
        }
        if (!repo->get_config().get_skip_if_unavailable_option().get_value()) {
            except_in_main_thread = true;
            finish_sack_loader();
            throw;
        }
        base->get_logger()->warning(
            "Error loading repo \"{}\" (skipping due to \"skip_if_unavailable=true\"):", repo->get_id());
        const auto & error_lines = utils::string::split(format(e, FormatDetailLevel::Plain), "\n");
        for (const auto & line : error_lines) {
            if (!line.empty()) {
                base->get_logger()->warning(' ' + line);
            }
        }
        return false;
    };

    std::vector<Repo *> repos_with_bad_signature;

    for (int run_count = 0; run_count < 2; ++run_count) {
        std::vector<Repo *> repos_for_processing;  // array of repositories for processing

        if (run_count == 0) {
            // First run.
            // If the input RepoQuery contains a system repo, we load the system repo first.
            // Available repos are stored to array of repositories for processing
            for (const auto & repo : repos) {
                switch (repo->get_type()) {
                    case Repo::Type::AVAILABLE:
                        repos_for_processing.emplace_back(repo.get());
                        break;
                    case Repo::Type::SYSTEM:
                        send_to_sack_loader(repo.get());
                        break;
                    case Repo::Type::COMMANDLINE:;
                }
            }
        } else {
            // Second run.
            // It will try to download and import keys for repositories with a bad signature.
            // Repositories with a bad signature are moved to the array of repositories for processing.

            if (!import_keys || repos_with_bad_signature.empty()) {
                break;
            }

            // Separates local and remote key files. Prepares local temporary files for storing downloaded remote keys.
            std::vector<std::tuple<Repo *, std::string>> local_keys_files;
            std::vector<std::tuple<Repo *, std::string, utils::fs::TempFile>> remote_keys_files;
            for (auto * repo : repos_with_bad_signature) {
                for (const auto & key_url : repo->config.get_gpgkey_option().get_value()) {
                    if (key_url.starts_with("file:/")) {
                        local_keys_files.emplace_back(repo, key_url);
                    } else {
                        auto & key_file = remote_keys_files.emplace_back(repo, key_url, "repokey");
                        auto & temp_file = std::get<2>(key_file);
                        temp_file.close();
                    }
                }
            }

            // import local keys files
            for (auto & [repo, key_url] : local_keys_files) {
                unsigned local_path_start_idx = key_url.starts_with("file:///") ? 7 : 5;
                utils::fs::File file(key_url.substr(local_path_start_idx), "r");
                repo->downloader->pgp.import_key(file.get_fd(), key_url);
            }

            if (!remote_keys_files.empty()) {
                // download remote keys files to local temporary files
                FileDownloader downloader(base);
                downloader.set_fail_fast(false);
                downloader.set_resume(false);

                for (const auto & [repo, key_url, key_file] : remote_keys_files) {
                    downloader.add(key_url, key_file.get_path(), repo->get_user_data());
                }

                downloader.download();

                // import downloaded keys files
                for (const auto & [repo, key_url, temp_file] : remote_keys_files) {
                    utils::fs::File file(temp_file.get_path(), "r");
                    repo->downloader->pgp.import_key(file.get_fd(), key_url);
                }
            }

            import_keys = false;

            repos_for_processing = std::move(repos_with_bad_signature);
        }

        std::string prev_repo_id;
        bool root_cache_tried = false;
        // Prepares repositories that are not expired or have ONLY_CACHE or LAZY SynStrategy.
        for (std::size_t idx = 0; idx < repos_for_processing.size();) {
            auto * const repo = repos_for_processing[idx];
            if (prev_repo_id != repo->get_id()) {
                prev_repo_id = repo->get_id();
                root_cache_tried = false;
            }
            catch_thread_sack_loader_exceptions();
            try {
                bool valid_metadata{false};
                try {
                    repo->read_metadata_cache();
                    if (!repo->downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).empty()) {
                        // cache loaded
                        repo->recompute_expired();
                        valid_metadata = !repo->expired || repo->sync_strategy == Repo::SyncStrategy::ONLY_CACHE ||
                                         repo->sync_strategy == Repo::SyncStrategy::LAZY;
                    }
                } catch (const std::runtime_error & e) {
                }

                if (valid_metadata) {
                    repos_for_processing.erase(repos_for_processing.begin() + static_cast<ssize_t>(idx));
                    logger->debug("Using cache for repo \"{}\"", repo->config.get_id());
                    send_to_sack_loader(repo);
                } else {
                    // Try reusing the root cache
                    if (!root_cache_tried && !libdnf5::utils::am_i_root() && repo->clone_root_metadata()) {
                        root_cache_tried = true;
                        continue;
                    }

                    if (repo->get_sync_strategy() == Repo::SyncStrategy::ONLY_CACHE) {
                        throw RepoDownloadError(
                            M_("Cache-only enabled but no cache for repository \"{}\""), repo->config.get_id());
                    }
                    ++idx;
                }

            } catch (const RepoDownloadError & e) {
                handle_repo_download_error(repo, e, false);
                repos_for_processing.erase(repos_for_processing.begin() + static_cast<ssize_t>(idx));
            } catch (const std::runtime_error & e) {
                except_in_main_thread = true;
                finish_sack_loader();
                throw;
            }
        }

        // Prepares repositories that are expired but match the original.
        for (std::size_t idx = 0; idx < repos_for_processing.size();) {
            auto * const repo = repos_for_processing[idx];
            catch_thread_sack_loader_exceptions();
            try {
                if (!repo->downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).empty() &&
                    repo->is_in_sync()) {
                    // the expired metadata still reflect the origin
                    utimes(repo->downloader->get_metadata_path(RepoDownloader::MD_FILENAME_PRIMARY).c_str(), nullptr);
                    RepoCache(base, repo->config.get_cachedir()).remove_attribute(RepoCache::ATTRIBUTE_EXPIRED);
                    repo->expired = false;
                    repos_for_processing.erase(repos_for_processing.begin() + static_cast<ssize_t>(idx));

                    logger->debug(
                        "Using cache for repo \"{}\". It is expired, but matches the original.", repo->config.get_id());
                    send_to_sack_loader(repo);
                } else {
                    ++idx;
                }

            } catch (const RepoDownloadError & e) {
                if (handle_repo_download_error(repo, e, import_keys)) {
                    repos_with_bad_signature.emplace_back(repo);
                }
                repos_for_processing.erase(repos_for_processing.begin() + static_cast<ssize_t>(idx));
            } catch (const std::runtime_error & e) {
                except_in_main_thread = true;
                finish_sack_loader();
                throw;
            }
        }

        // Prepares (downloads) remaining repositories.
        for (std::size_t idx = 0; idx < repos_for_processing.size();) {
            auto * const repo = repos_for_processing[idx];
            catch_thread_sack_loader_exceptions();
            try {
                logger->debug("Downloading metadata for repo \"{}\"", repo->config.get_id());
                auto cache_dir = repo->config.get_cachedir();
                repo->download_metadata(cache_dir);
                RepoCache(base, cache_dir).remove_attribute(RepoCache::ATTRIBUTE_EXPIRED);
                repo->read_metadata_cache();
                repo->expired = false;

                repos_for_processing.erase(repos_for_processing.begin() + static_cast<ssize_t>(idx));
                send_to_sack_loader(repo);
            } catch (const RepoDownloadError & e) {
                if (handle_repo_download_error(repo, e, import_keys)) {
                    repos_with_bad_signature.emplace_back(repo);
                }
                repos_for_processing.erase(repos_for_processing.begin() + static_cast<ssize_t>(idx));
            } catch (const std::runtime_error & e) {
                except_in_main_thread = true;
                finish_sack_loader();
                throw;
            }
        }
    };

    finish_sack_loader();
    catch_thread_sack_loader_exceptions();

    fix_group_missing_xml();

    base->get_rpm_package_sack()->load_config_excludes_includes();
}


void RepoSack::update_and_load_enabled_repos(bool load_system) {
    libdnf_user_assert(!repos_updated_and_loaded, "RepoSack::updated_and_load_enabled_repos has already been called.");

    if (load_system) {
        // create the system repository if it does not exist
        base->get_repo_sack()->get_system_repo();
    }

    libdnf5::repo::RepoQuery repos(base);
    repos.filter_enabled(true);

    if (!load_system) {
        repos.filter_type(Repo::Type::SYSTEM, libdnf5::sack::QueryCmp::NEQ);
    }

    update_and_load_repos(repos);

    // TODO(jmracek) Replace by call that will resolve active modules and apply modular filtering
    base->get_module_sack()->p_impl->module_filtering();

    repos_updated_and_loaded = true;
}


void RepoSack::dump_debugdata(const std::string & dir) {
    libdnf5::solv::Solver solver{get_rpm_pool(base)};
    solver.write_debugdata(dir, false);
}


void RepoSack::dump_comps_debugdata(const std::string & dir) {
    libdnf5::solv::Solver solver{get_comps_pool(base)};
    solver.write_debugdata(dir, false);
}


void RepoSack::create_repos_from_file(const std::string & path) {
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

        logger.debug("Creating repo \"{}\" from config file \"{}\" section \"{}\"", repo_id, path, section);

        RepoWeakPtr repo;
        try {
            repo = create_repo(repo_id);
        } catch (const RepoIdAlreadyExistsError & ex) {
            logger.error(ex.what());
            continue;
        }
        repo->set_repo_file_path(path);
        auto & repo_cfg = repo->get_config();
        repo_cfg.load_from_parser(parser, section, *base->get_vars(), *base->get_logger());

        if (repo_cfg.get_name_option().get_priority() == Option::Priority::DEFAULT) {
            logger.debug("Repo \"{}\" is missing name in configuration file \"{}\", using id.", repo_id, path);
            repo_cfg.get_name_option().set(Option::Priority::REPOCONFIG, repo_id);
        }
    }
}

void RepoSack::create_repos_from_config_file() {
    base->with_config_file_path(
        std::function<void(const std::string &)>{[this](const std::string & path) { create_repos_from_file(path); }});
}

void RepoSack::create_repos_from_dir(const std::string & dir_path) {
    std::error_code ec;
    std::filesystem::directory_iterator di(dir_path, ec);
    if (ec) {
        base->get_logger()->warning("Cannot read repositories from directory \"{}\": {}", dir_path, ec.message());
        return;
    }
    std::vector<std::filesystem::path> paths;
    for (auto & dentry : di) {
        auto & path = dentry.path();
        if (dentry.is_regular_file() && path.extension() == ".repo") {
            paths.push_back(path);
        }
    }
    std::sort(paths.begin(), paths.end());
    for (auto & path : paths) {
        create_repos_from_file(path);
    }
}

void RepoSack::create_repos_from_reposdir() {
    for (auto & dir : base->get_config().get_reposdir_option().get_value()) {
        if (std::filesystem::exists(dir)) {
            create_repos_from_dir(dir);
        }
    }
}

void RepoSack::create_repos_from_paths(
    const std::vector<std::pair<std::string, std::string>> & repos_paths, libdnf5::Option::Priority priority) {
    for (const auto & [id, path] : repos_paths) {
        auto new_repo = create_repo(id);
        auto & new_repo_config = new_repo->get_config();
        new_repo_config.get_name_option().set(priority, id);
        new_repo_config.get_baseurl_option().set(priority, path);
    }
}

void RepoSack::create_repos_from_system_configuration() {
    create_repos_from_config_file();
    create_repos_from_reposdir();
    load_repos_configuration_overrides();
}

void RepoSack::load_repos_configuration_overrides() {
    auto loger = base->get_logger();

    fs::path repos_override_dir_path{REPOS_OVERRIDE_DIR};
    fs::path repos_distrib_override_dir_path{LIBDNF5_REPOS_DISTRIBUTION_OVERRIDE_DIR};

    const auto & config = base->get_config();
    const bool use_installroot_config{!config.get_use_host_config_option().get_value()};
    if (use_installroot_config) {
        const fs::path installroot_path{config.get_installroot_option().get_value()};
        repos_override_dir_path = installroot_path / repos_override_dir_path.relative_path();
        repos_distrib_override_dir_path = installroot_path / repos_distrib_override_dir_path.relative_path();
    }

    const auto paths =
        utils::fs::create_sorted_file_list({repos_override_dir_path, repos_distrib_override_dir_path}, ".repo");

    for (const auto & path : paths) {
        ConfigParser parser;
        parser.read(path);
        const auto & cfg_parser_data = parser.get_data();
        for (const auto & cfg_parser_data_iter : cfg_parser_data) {
            const auto & section = cfg_parser_data_iter.first;
            const auto repo_id_pattern = base->get_vars()->substitute(section);

            RepoQuery repo_query(base);
            repo_query.filter_id(repo_id_pattern, sack::QueryCmp::GLOB);
            for (auto & repo : repo_query) {
                repo->get_config().load_from_parser(parser, section, *base->get_vars(), *base->get_logger());
            }
        }
    }
}

BaseWeakPtr RepoSack::get_base() const {
    return base;
}

void RepoSack::enable_source_repos() {
    RepoQuery enabled_repos(base);
    enabled_repos.filter_enabled(true);
    for (const auto & repo : enabled_repos) {
        RepoQuery source_query(base);
        // There is no way how to find source (or debuginfo) repository for
        // given repo. This is only guessing according to the current practice:
        auto repoid = repo->get_id();
        if (libdnf5::utils::string::ends_with(repoid, "-rpms")) {
            source_query.filter_id(repoid.substr(0, repoid.size() - 5) + "-source-rpms");
        } else {
            source_query.filter_id(repoid + "-source");
        }
        for (auto & source_repo : source_query) {
            if (!source_repo->is_enabled()) {
                // TODO(mblaha): log source repo enabling
                source_repo->enable();
            }
        }
    }
}

void RepoSack::internalize_repos() {
    auto rq = RepoQuery(base);
    for (auto & repo : rq.get_data()) {
        repo->internalize();
    }

    if (system_repo) {
        system_repo->internalize();
    }

    if (cmdline_repo) {
        cmdline_repo->internalize();
    }
}

void RepoSack::fix_group_missing_xml() {
    if (has_system_repo()) {
        auto & solv_repo = system_repo->solv_repo;
        auto & group_missing_xml = solv_repo->get_groups_missing_xml();
        auto & environments_missing_xml = solv_repo->get_environments_missing_xml();
        if (group_missing_xml.empty() && environments_missing_xml.empty()) {
            return;
        }
        auto & logger = *base->get_logger();
        auto & system_state = base->p_impl->get_system_state();
        auto comps_xml_dir = system_state.get_group_xml_dir();
        bool directory_exists = true;
        std::error_code ec;
        std::filesystem::create_directories(comps_xml_dir, ec);
        if (ec) {
            logger.debug("Failed to create directory \"{}\": {}", comps_xml_dir.string(), ec.message());
            directory_exists = false;
        }
        if (!group_missing_xml.empty()) {
            libdnf5::comps::GroupQuery available_groups(base);
            available_groups.filter_installed(false);
            for (const auto & group_id : group_missing_xml) {
                bool xml_saved = false;
                if (directory_exists) {
                    // try to find the group id in availables
                    libdnf5::comps::GroupQuery group_query(available_groups);
                    group_query.filter_groupid(group_id);
                    if (group_query.size() == 1) {
                        // GroupQuery is basically a set thus iterators and `.get()` method
                        // return `const Group` objects.
                        // To call non-const serialize method we need to make a copy here.
                        libdnf5::comps::Group group = group_query.get();
                        auto xml_file_name = comps_xml_dir / (group_id + ".xml");
                        logger.debug(
                            "Re-creating installed group \"{}\" definition to file \"{}\".",
                            group_id,
                            xml_file_name.string());
                        try {
                            group.serialize(xml_file_name);
                            xml_saved = true;
                        } catch (utils::xml::XMLSaveError & ex) {
                            logger.debug(ex.what());
                        }
                        if (xml_saved) {
                            solv_repo->read_group_solvable_from_xml(xml_file_name);
                        }
                    }
                }
                if (!xml_saved) {
                    // fall-back to creating solvables only from system state
                    solv_repo->create_group_solvable(group_id, system_state.get_group_state(group_id));
                }
            }
        }
        if (!environments_missing_xml.empty()) {
            libdnf5::comps::EnvironmentQuery available_environments(base);
            available_environments.filter_installed(false);
            for (const auto & environment_id : environments_missing_xml) {
                bool xml_saved = false;
                if (directory_exists) {
                    // try to find the environment id in availables
                    libdnf5::comps::EnvironmentQuery environment_query(available_environments);
                    environment_query.filter_environmentid(environment_id);
                    if (environment_query.size() == 1) {
                        libdnf5::comps::Environment environment = environment_query.get();
                        auto xml_file_name = comps_xml_dir / (environment_id + ".xml");
                        logger.debug(
                            "Re-creating installed environmental group \"{}\" definition to file \"{}\".",
                            environment_id,
                            xml_file_name.string());
                        try {
                            environment.serialize(xml_file_name);
                            xml_saved = true;
                        } catch (utils::xml::XMLSaveError & ex) {
                            logger.debug(ex.what());
                        }
                        if (xml_saved) {
                            solv_repo->read_group_solvable_from_xml(xml_file_name);
                        }
                    }
                }
                if (!xml_saved) {
                    // fall-back to creating solvables only from system state
                    solv_repo->create_environment_solvable(
                        environment_id, system_state.get_environment_state(environment_id));
                }
            }
        }

        // ensure we attempt to re-create xmls only once
        group_missing_xml.clear();
        environments_missing_xml.clear();
    }
}

}  // namespace libdnf5::repo
