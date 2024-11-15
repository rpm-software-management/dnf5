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


#ifndef DNF5_PLUGINS_REPOSYNC_PLUGIN_REPOSYNC_HPP
#define DNF5_PLUGINS_REPOSYNC_PLUGIN_REPOSYNC_HPP


#include <dnf5/context.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/repo/repo.hpp>
#include <libdnf5/rpm/package.hpp>

#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

namespace dnf5 {


class ReposyncCommand : public Command {
public:
    explicit ReposyncCommand(Context & context) : Command(context, "reposync") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    using download_list_type = std::vector<std::pair<std::filesystem::path, libdnf5::rpm::Package>>;

    std::unique_ptr<libdnf5::cli::session::BoolOption> newest_option{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> remote_time_option{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> norepopath_option{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> delete_option{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> urls_option{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> gpgcheck_option{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> download_metadata_option{nullptr};
    std::unordered_set<std::string> arch_option;
    libdnf5::OptionString * safe_write_path_option{nullptr};
    libdnf5::OptionString * metadata_path_option{nullptr};

    std::filesystem::path repo_download_path(const libdnf5::repo::Repo & repo);
    void limit_to_latest(libdnf5::rpm::PackageQuery & query);
    download_list_type get_packages_list(const libdnf5::repo::Repo & repo);
    void download_packages(const download_list_type & pkg_list);
    void delete_old_local_packages(const libdnf5::repo::Repo & repo, const download_list_type & pkg_list);
    bool pgp_check_packages(const download_list_type & pkg_list);
    void download_metadata(libdnf5::repo::Repo & repo);
};


}  // namespace dnf5


#endif  // DNF5_PLUGINS_REPOSYNC_PLUGIN_REPOSYNC_HPP
