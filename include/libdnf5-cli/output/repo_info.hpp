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


#ifndef LIBDNF5_CLI_OUTPUT_REPO_INFO_HPP
#define LIBDNF5_CLI_OUTPUT_REPO_INFO_HPP


#include "fmt/chrono.h"
#include "key_value_table.hpp"

#include "libdnf5-cli/utils/units.hpp"


namespace libdnf5::cli::output {


class RepoInfo : public KeyValueTable {
public:
    template <typename Repo>
    void add_repo(Repo & repo);
};


template <typename Repo>
void RepoInfo::add_repo(Repo & repo) {
    add_line("Repo ID", repo.get_id(), "bold");
    add_line("Name", repo.get_name());

    add_line("Status", repo.is_enabled() ? "enabled" : "disabled", repo.is_enabled() ? "green" : "red");

    add_line("Priority", repo.get_priority());
    add_line("Cost", repo.get_cost());
    add_line("Type", repo.get_type());
    add_line("Exclude packages", repo.get_excludepkgs());
    add_line("Include packages", repo.get_includepkgs());
    add_line("Metadata expire", fmt::format("{} seconds", repo.get_metadata_expire()));
    add_line("Skip if unavailable", fmt::format("{}", repo.get_skip_if_unavailable()));
    add_line("Config file", repo.get_repo_file_path());

    // URLs
    auto group_urls = add_line("URLs", "", nullptr);
    add_line("Base URL", repo.get_baseurl(), nullptr, group_urls);
    add_line("Mirrorlist", repo.get_mirrorlist(), nullptr, group_urls);
    add_line("Metalink", repo.get_metalink(), nullptr, group_urls);

    // PGP
    auto group_gpg = add_line("PGP", "", nullptr);
    add_line("Keys", repo.get_gpgkey(), nullptr, group_gpg);
    add_line("Verify repodata", fmt::format("{}", repo.get_repo_gpgcheck()), nullptr, group_gpg);
    add_line("Verify packages", fmt::format("{}", repo.get_gpgcheck()), nullptr, group_gpg);

    // TODO(jkolarik): Verbose is not implemented and not used yet
    // if (verbose) {
    //     // Connection settings
    //     auto group_conn = add_line("Connection settings", "");
    //     add_line("Authentication method", "", nullptr, group_conn);
    //     add_line("Username", "", nullptr, group_conn);
    //     add_line("Password", "", nullptr, group_conn);
    //     add_line("SSL CA certificate", "", nullptr, group_conn);
    //     add_line("SSL client certificate", "", nullptr, group_conn);
    //     add_line("SSL client key", "", nullptr, group_conn);
    //     add_line("Verify SSL certificate", "", nullptr, group_conn);

    //     // Proxy settings
    //     auto group_proxy = add_line("Proxy settings", "");
    //     add_line("URL", "", nullptr, group_proxy);
    //     add_line("Authentication method", "", nullptr, group_proxy);
    //     add_line("Username", "", nullptr, group_proxy);
    //     add_line("Password", "", nullptr, group_proxy);
    //     add_line("SSL CA certificate", "", nullptr, group_proxy);
    //     add_line("SSL client certificate", "", nullptr, group_proxy);
    //     add_line("SSL client key", "", nullptr, group_proxy);
    //     add_line("Verify SSL certificate", "", nullptr, group_proxy);

    //     auto group_misc = add_line("Miscelaneous", "");
    //     add_line("Load comps groups", "", nullptr, group_misc);
    //     add_line("Report \"countme\" statistics", "", nullptr, group_misc);
    //     add_line("Enable DeltaRPM", "", nullptr, group_misc);
    //     add_line("DeltaRPM percentage", "", nullptr, group_misc);
    //     add_line("Use the fastest mirror", "", nullptr, group_misc);
    //     add_line("Repository provides module hotfixes", "", nullptr, group_misc);
    //     add_line("Use zchunk repodata", "", nullptr, group_misc);
    // }

    // Repodata
    auto group_repodata = add_line("Repodata info", "", nullptr);
    add_line("Available packages", repo.get_available_pkgs(), nullptr, group_repodata);
    add_line("Total packages", repo.get_pkgs(), nullptr, group_repodata);

    std::string size = libdnf5::cli::utils::units::format_size_aligned(static_cast<int64_t>(repo.get_size()));
    add_line("Size", size, nullptr, group_repodata);

    add_line("Content tags", repo.get_content_tags(), nullptr, group_repodata);

    std::vector<std::string> distro_tags;
    for (auto & key_value : repo.get_distro_tags()) {
        distro_tags.push_back(key_value.second + " (" + key_value.first + ")");
    }
    add_line("Distro tags", distro_tags, nullptr, group_repodata);
    add_line("Revision", repo.get_revision(), nullptr, group_repodata);

    const auto updated_time = static_cast<time_t>(repo.get_max_timestamp());
    add_line(
        "Cache updated",
        fmt::format("{:%F %X}", std::chrono::system_clock::from_time_t(updated_time)),
        nullptr,
        group_repodata);

    /*
general connection settings?
       bandwidth
       ip_resolve
       max_parallel_downloads
       minrate
       retries
       throttle
       timeout
       user_agent
*/
}


}  // namespace libdnf5::cli::output


#endif  // LIBDNF5_CLI_OUTPUT_REPO_INFO_HPP
