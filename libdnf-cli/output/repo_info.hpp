/*
Copyright (C) 2021 Red Hat, Inc.

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


#ifndef LIBDNF_CLI_OUTPUT_REPO_INFO_HPP
#define LIBDNF_CLI_OUTPUT_REPO_INFO_HPP


#include "key_value_table.hpp"
#include "libdnf-cli/utils/units.hpp"


namespace libdnf::cli::output {


class RepoInfo : public KeyValueTable {
public:
    template <typename Repo>
    void add_repo(const Repo & repo, bool verbose = false, bool show_sack_data = false);
};


template <typename Repo>
void RepoInfo::add_repo(const Repo & repo, bool verbose, bool show_sack_data) {
    add_line("Repo ID", repo.get_id(), "bold");
    add_line("Name", repo.get_name());

    add_line(
        "Status",
        repo.is_enabled() ? "enabled" : "disabled",
        repo.is_enabled() ? "green" : "red"
    );

    add_line("Priority", "");
    add_line("Cost", "");
    add_line("Type", "");
    add_line("Exclude packages", repo.get_excludepkgs());
    add_line("Include packages", repo.get_includepkgs());
    add_line("Metadata expire", "");
    add_line("Skip if unavailable", "");
    add_line("Config file", repo.get_repofile());

    // URLs
    auto group_urls = add_line("URLs", "", nullptr);
    add_line("Base URL", repo.get_baseurl(), nullptr, group_urls);
    add_line("Mirrorlist", repo.get_mirrorlist(), nullptr, group_urls);
    add_line("Metalink", repo.get_metalink(), nullptr, group_urls);

    // GPG
    auto group_gpg = add_line("GPG", "", nullptr);
    add_line("Keys", "", nullptr, group_gpg);
    add_line("Verify repodata", "", nullptr, group_gpg);
    add_line("Verify packages", "", nullptr, group_gpg);

    if (verbose) {

        // Connection settings
        auto group_conn = add_line("Connection settings", "");
        add_line("Authentication method", "", nullptr, group_conn);
        add_line("Username", "", nullptr, group_conn);
        add_line("Password", "", nullptr, group_conn);
        add_line("SSL CA certificate", "", nullptr, group_conn);
        add_line("SSL client certificate", "", nullptr, group_conn);
        add_line("SSL client key", "", nullptr, group_conn);
        add_line("Verify SSL certificate", "", nullptr, group_conn);

        // Proxy settings
        auto group_proxy = add_line("Proxy settings", "");
        add_line("URL", "", nullptr, group_proxy);
        add_line("Authentication method", "", nullptr, group_proxy);
        add_line("Username", "", nullptr, group_proxy);
        add_line("Password", "", nullptr, group_proxy);
        add_line("SSL CA certificate", "", nullptr, group_proxy);
        add_line("SSL client certificate", "", nullptr, group_proxy);
        add_line("SSL client key", "", nullptr, group_proxy);
        add_line("Verify SSL certificate", "", nullptr, group_proxy);

        auto group_misc = add_line("Miscelaneous", "");
        add_line("Load comps groups", "", nullptr, group_misc);
        add_line("Report \"countme\" statistics", "", nullptr, group_misc);
        add_line("Enable DeltaRPM", "", nullptr, group_misc);
        add_line("DeltaRPM percentage", "", nullptr, group_misc);
        add_line("Use the fastest mirror", "", nullptr, group_misc);
        add_line("Repository provides module hotfixes", "", nullptr, group_misc);
        add_line("Use zchunk repodata", "", nullptr, group_misc);
    }

    if (show_sack_data) {
        // the following lines require downloaded repodata loaded into sack
        auto group_repodata = add_line("Repodata info", "", nullptr);
        add_line("Available packages", repo.get_available_pkgs(), nullptr, group_repodata);
        add_line("Total packages", repo.get_pkgs(), nullptr, group_repodata);

        std::string size = libdnf::cli::utils::units::format_size(static_cast<int64_t>(repo.get_size()));
        add_line("Size", size, nullptr, group_repodata);

        add_line("Content tags", repo.get_content_tags(), nullptr, group_repodata);

        std::vector<std::string> distro_tags;
        for (auto & key_value : repo.get_distro_tags()) {
            distro_tags.push_back(key_value.second + " (" + key_value.first + ")");
        }
        add_line("Distro tags", distro_tags, nullptr, group_repodata);
        add_line("Revision", repo.get_revision(), nullptr, group_repodata);
        add_line("Cache updated", repo.get_max_timestamp(), nullptr, group_repodata);
    }

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


}  // namespace libdnf::cli::output


#endif  // LIBDNF_CLI_OUTPUT_REPO_INFO_HPP
