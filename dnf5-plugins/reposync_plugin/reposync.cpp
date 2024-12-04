/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "reposync.hpp"

#include <libdnf5-cli/exception.hpp>
#include <libdnf5/common/sack/exclude_flags.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/conf/option_string_list.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/repo/repo_query.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/rpm_signature.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <string>
#include <string_view>
#include <vector>

namespace {

static std::string join_url(const std::string & base, const std::string & path) {
    if (base.back() == '/' && path.front() == '/') {
        return base + path.substr(1);
    } else if (base.back() != '/' && path.front() != '/') {
        return base + "/" + path;
    } else {
        return base + path;
    }
}

}  // namespace

namespace dnf5 {

void ReposyncCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void ReposyncCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Synchronize a remote DNF repository to a local directory."));

    auto * arch_arg = parser.add_new_named_arg("arch");
    arch_arg->set_long_name("arch");
    arch_arg->set_short_name('a');
    arch_arg->set_description("Limit downloaded packages to given architectures");
    arch_arg->set_has_value(true);
    arch_arg->set_arg_value_help("<ARCH>,...");
    arch_arg->set_parse_hook_func([this](
                                      [[maybe_unused]] libdnf5::cli::ArgumentParser::NamedArg * arg,
                                      [[maybe_unused]] const char * option,
                                      const char * value) {
        const libdnf5::OptionStringList list_value(value);
        for (const auto & arch : list_value.get_value()) {
            arch_option.emplace(arch);
        }
        return true;
    });
    cmd.register_named_arg(arch_arg);

    auto * srpm_arg = parser.add_new_named_arg("srpm");
    srpm_arg->set_long_name("srpm");
    srpm_arg->set_description("Download source packages");
    srpm_arg->set_has_value(false);
    srpm_arg->set_parse_hook_func([this](
                                      [[maybe_unused]] libdnf5::cli::ArgumentParser::NamedArg * arg,
                                      [[maybe_unused]] const char * option,
                                      [[maybe_unused]] const char * value) {
        arch_option.emplace("src");
        return true;
    });
    cmd.register_named_arg(srpm_arg);

    newest_option = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "newest-only", 'n', "Download only newest packages per-repo", false);

    remote_time_option = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "remote-time", '\0', "Set timestamps of the downloaded files according to remote side", false);

    norepopath_option = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "norepopath", '\0', "Don't add the reponame to the download path", false);
    delete_option = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "delete", '\0', "Delete local packages no longer present in repository", false);

    urls_option = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "urls", 'u', "Print URLs where the rpms can be downloaded instead of downloading", false);

    gpgcheck_option = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "gpgcheck", 'g', "Remove packages that fail GPG signature checking after downloading", false);

    download_metadata_option = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "download-metadata", '\0', "Download all repository metadata", false);

    auto * destdir_arg = parser.add_new_named_arg("destdir");
    destdir_arg->set_long_name("destdir");
    destdir_arg->set_description("Root path under which the downloaded repositories are stored");
    destdir_arg->set_has_value(true);
    destdir_arg->set_arg_value_help("<DIR>");
    destdir_arg->link_value(&ctx.get_base().get_config().get_destdir_option());
    cmd.register_named_arg(destdir_arg);

    safe_write_path_option =
        dynamic_cast<libdnf5::OptionString *>(parser.add_init_value(std::make_unique<libdnf5::OptionString>("")));
    auto safe_write_path_arg = parser.add_new_named_arg("safe_write_path");
    safe_write_path_arg->set_long_name("safe-write-path");
    safe_write_path_arg->set_description("Filesystem path considered safe for writing");
    safe_write_path_arg->set_has_value(true);
    safe_write_path_arg->set_arg_value_help("<DIR>");
    safe_write_path_arg->link_value(safe_write_path_option);
    cmd.register_named_arg(safe_write_path_arg);

    metadata_path_option =
        dynamic_cast<libdnf5::OptionString *>(parser.add_init_value(std::make_unique<libdnf5::OptionString>("")));
    auto metadata_path_arg = parser.add_new_named_arg("metadata_path");
    metadata_path_arg->set_long_name("metadata-path");
    metadata_path_arg->set_description("Root path under which the downloaded metadata are stored");
    metadata_path_arg->set_has_value(true);
    metadata_path_arg->set_arg_value_help("<DIR>");
    metadata_path_arg->link_value(metadata_path_option);
    cmd.register_named_arg(metadata_path_arg);
}

void ReposyncCommand::configure() {
    auto & context = get_context();
    auto & base = context.get_base();
    if (arch_option.contains("src")) {
        base.get_repo_sack()->enable_source_repos();
    }

    libdnf5::repo::RepoQuery repos_query(base);
    repos_query.filter_enabled(true);

    if (norepopath_option->get_value() && repos_query.size() > 1) {
        throw libdnf5::cli::ArgumentParserConflictingArgumentsError(
            M_("Can't use --norepopath with multiple repositories enabled"));
    }

    // Default destination for downloaded repos is the current directory
    context.get_base().get_config().get_destdir_option().set(libdnf5::Option::Priority::DEFAULT, ".");

    const bool preserve_remote_time = remote_time_option->get_value();
    for (const auto & repo : repos_query) {
        repo->set_preserve_remote_time(preserve_remote_time);
        // expire all the enabled repos before downloading to ensure that the fresh
        // metadata are used.
        repo->expire();
    }

    context.set_load_system_repo(false);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}


std::filesystem::path ReposyncCommand::repo_download_path(const libdnf5::repo::Repo & repo) {
    // first convert the destdir to the absolute path
    std::filesystem::path repo_path =
        std::filesystem::absolute(get_context().get_base().get_config().get_destdir_option().get_value());
    if (!norepopath_option->get_value()) {
        repo_path /= repo.get_id();
    }
    // resolve '.', '..', and existing symlinks in the repo_path
    return std::filesystem::weakly_canonical(repo_path);
}

void ReposyncCommand::limit_to_latest(libdnf5::rpm::PackageQuery & query) {
    // TODO(mblaha): implement modularity support
    // https://github.com/rpm-software-management/dnf5/issues/1902
    // Returned query should contain a union of these queries:
    // - the latest NEVRAs from non-modular packages
    // - all packages from stream version with the latest package NEVRA
    //   (this should not be needed but the latest package NEVRAs might be
    //   part of an older module version)
    // - all packages from the latest stream version

    query.filter_latest_evr();
}

ReposyncCommand::download_list_type ReposyncCommand::get_packages_list(const libdnf5::repo::Repo & repo) {
    auto & ctx = get_context();
    download_list_type result;

    const auto repo_path = repo_download_path(repo);

    // Safe path is either repository download path or --safe-write-path option value.
    std::filesystem::path safe_write_path;
    if (!safe_write_path_option->get_value().empty()) {
        safe_write_path =
            std::filesystem::weakly_canonical(std::filesystem::absolute(safe_write_path_option->get_value()));
    } else {
        safe_write_path = repo_path;
    }
    // Ensure the safe write path ends with a directory separator by appending
    // an empty path.
    // The download location is validated to ensure it resides within this safe
    // path by checking that the download path string starts with the safe
    // path. To avoid false positives, we ensure the safe path ends with a
    // directory separator. For example, if "/tmp/path" is the safe path,
    // "/tmp/path2/evil" would incorrectly match as within the safe path unless
    // a separator is enforced.
    safe_write_path /= "";

    libdnf5::rpm::PackageQuery query(ctx.get_base(), libdnf5::sack::ExcludeFlags::IGNORE_MODULAR_EXCLUDES);
    query.filter_available();
    query.filter_repo_id(repo.get_id());

    if (newest_option->get_value()) {
        limit_to_latest(query);
    }

    if (!arch_option.empty()) {
        query.filter_arch(std::vector<std::string>(arch_option.begin(), arch_option.end()));
    }

    for (auto pkg : query) {
        auto pkg_path = std::filesystem::weakly_canonical(std::filesystem::absolute(repo_path / pkg.get_location()));

        // check that the location is safe
        if (!pkg_path.string().starts_with(safe_write_path.c_str())) {
            throw libdnf5::cli::CommandExitError(
                1,
                M_("Download destination '{0}' for location '{1}' of '{2}' package from '{3}' repo is outside of safe "
                   "write path '{4}'."),
                pkg_path.string(),
                pkg.get_location(),
                pkg.get_full_nevra(),
                repo.get_id(),
                safe_write_path.string());
        }
        // std::map assures that duplicated packages with the same download
        // path are skipped
        result.emplace(std::move(pkg_path), std::move(pkg));
    }

    return result;
}

void ReposyncCommand::download_packages(const ReposyncCommand::download_list_type & pkg_list) {
    libdnf5::repo::PackageDownloader downloader(get_context().get_base());
    downloader.force_keep_packages(true);
    // do not stop on the first error but download as much packages as available
    downloader.set_fail_fast(false);
    for (const auto & [pth, pkg] : pkg_list) {
        downloader.add(pkg, pth.parent_path());
    }
    downloader.download();
    // TODO(mblaha): Return exit code 1 if any of packages was not downloaded.
    // In case of fail_fast set to false, the download() method does
    // not throw an exception if particular package could not be downloaded.
    // See https://github.com/rpm-software-management/dnf5/issues/1926 for details
}

void ReposyncCommand::delete_old_local_packages(
    const libdnf5::repo::Repo & repo, const ReposyncCommand::download_list_type & pkg_list) {
    const auto repo_path = repo_download_path(repo);

    std::error_code ec;
    std::filesystem::recursive_directory_iterator delete_iterator(repo_path, ec);
    if (ec) {
        std::cerr << libdnf5::utils::sformat(
                         _("Failed to create directory '{0}' iterator: {1}"), repo_path.string(), ec.message())
                  << std::endl;
        return;
    }

    for (const auto & entry : delete_iterator) {
        if (entry.is_regular_file(ec)) {
            const auto & file_path = entry.path();
            if (file_path.extension() == ".rpm" && !pkg_list.contains(file_path)) {
                // Remove every *.rpm file that was not downloaded from the repo
                std::filesystem::remove(file_path, ec);
                if (ec) {
                    std::cerr << libdnf5::utils::sformat(
                                     _("Failed to delete file {0}: {1}"), file_path.string(), ec.message())
                              << std::endl;
                } else {
                    std::cout << libdnf5::utils::sformat(_("[DELETED] {}"), file_path.string()) << std::endl;
                }
            }
        }
    }
}

bool ReposyncCommand::pgp_check_packages(const download_list_type & pkg_list) {
    bool ret = true;
    std::error_code ec;
    libdnf5::rpm::RpmSignature rpm_signature(get_context().get_base());
    for (const auto & [pth, pkg] : pkg_list) {
        if (std::filesystem::exists(pth, ec)) {
            auto check_result = rpm_signature.check_package_signature(pth);
            if (check_result != libdnf5::rpm::RpmSignature::CheckResult::OK) {
                std::cerr << libdnf5::utils::sformat(
                                 _("Removing '{}' with failing PGP check: {}"),
                                 pth.string(),
                                 rpm_signature.check_result_to_string(check_result))
                          << std::endl;
                std::filesystem::remove(pth, ec);
                ret = false;
            }
        }
    }
    return ret;
}

void ReposyncCommand::download_metadata(libdnf5::repo::Repo & repo) {
    std::filesystem::path repo_path;
    const auto metadata_path = metadata_path_option->get_value();
    if (!metadata_path.empty()) {
        repo_path = std::filesystem::absolute(metadata_path);
        if (!norepopath_option->get_value()) {
            repo_path /= repo.get_id();
        }
        // resolve '.', '..', and existing symlinks in the repo_path
        repo_path = std::filesystem::weakly_canonical(repo_path);
    } else {
        repo_path = repo_download_path(repo);
    }
    auto & optional_metadata_option = get_context().get_base().get_config().get_optional_metadata_types_option();
    if (!optional_metadata_option.get_value().contains(libdnf5::METADATA_TYPE_ALL)) {
        optional_metadata_option.set(libdnf5::METADATA_TYPE_ALL);
    }
    repo.download_metadata(repo_path);
}

void ReposyncCommand::run() {
    auto & context = get_context();
    libdnf5::repo::RepoQuery repos_query(context.get_base());
    repos_query.filter_enabled(true);
    std::vector<std::string_view> schemes{"https://", "file://", "http://", "ftp://"};
    for (const auto & repo : repos_query) {
        const auto pkg_list = get_packages_list(*repo);
        if (urls_option->get_value()) {
            if (download_metadata_option->get_value()) {
                // get list of repository remote locations (mirrors + base_url)
                std::vector<std::string> remote_locations;
                for (const auto & mirror : repo->get_mirrors()) {
                    remote_locations.emplace_back(mirror);
                }
                for (const auto & base_url : repo->get_config().get_baseurl_option().get_value()) {
                    remote_locations.emplace_back(base_url);
                }
                // find first available mirror prefering file and https schemes
                std::string repo_location{};
                for (const auto & scheme : schemes) {
                    for (const auto & mirror : remote_locations) {
                        if (mirror.starts_with(scheme)) {
                            repo_location = mirror;
                            break;
                        }
                    }
                    if (!repo_location.empty()) {
                        break;
                    }
                }
                if (repo_location.empty()) {
                    std::cerr << libdnf5::utils::sformat(_("Failed to get mirror for metadata.")) << std::endl;
                    continue;
                }
                for (const auto & [md_type, md_location] : repo->get_metadata_locations()) {
                    std::cout << join_url(repo_location, md_location) << std::endl;
                }
            }
            for (const auto & [pth, pkg] : pkg_list) {
                auto urls = pkg.get_remote_locations();
                if (urls.empty()) {
                    std::cerr << libdnf5::utils::sformat(_("Failed to get mirror for package: \"{}\""), pkg.get_name())
                              << std::endl;
                } else {
                    std::cout << urls[0] << std::endl;
                }
            }
        } else {
            if (download_metadata_option->get_value()) {
                download_metadata(*repo);
            }
            download_packages(pkg_list);
            if (delete_option->get_value()) {
                delete_old_local_packages(*repo, pkg_list);
            }
            if (gpgcheck_option->get_value()) {
                if (!pgp_check_packages(pkg_list)) {
                    throw libdnf5::cli::CommandExitError(1, M_("PGP signature check failed"));
                }
            }
        }
    }
}

}  // namespace dnf5
