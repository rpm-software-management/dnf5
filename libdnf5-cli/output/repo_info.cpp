// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5-cli/output/repo_info.hpp"

#include "key_value_table.hpp"
#include "utils/string.hpp"

#include "libdnf5-cli/utils/units.hpp"

#include <json-c/json_object.h>

#include <iostream>

namespace libdnf5::cli::output {

namespace {

std::vector<std::string> flatten_distro_tags(const std::vector<std::pair<std::string, std::string>> & distro_tags) {
    std::vector<std::string> distro_tags_flat;
    for (auto & key_value : distro_tags) {
        distro_tags_flat.push_back(key_value.second + " (" + key_value.first + ")");
    }
    return distro_tags_flat;
}

}  // namespace

class RepoInfo::Impl : public KeyValueTable {
public:
    void add_repo(IRepoInfo & repo);
};

void RepoInfo::Impl::add_repo(IRepoInfo & repo) {
    auto enabled = repo.is_enabled();

    add_line("Repo ID", repo.get_id(), "bold");
    add_line("Name", repo.get_name());

    add_line("Status", enabled ? "enabled" : "disabled", enabled ? "green" : "red");

    add_line("Priority", repo.get_priority());
    add_line("Cost", repo.get_cost());
    add_line("Type", repo.get_type());

    auto exclude_packages = repo.get_excludepkgs();
    if (!exclude_packages.empty()) {
        add_line("Exclude packages", exclude_packages);
    }

    auto include_packages = repo.get_includepkgs();
    if (!include_packages.empty()) {
        add_line("Include packages", include_packages);
    }

    auto cache_updated = repo.get_timestamp();
    std::string last_update = "unknown";
    if (cache_updated > 0) {
        last_update = libdnf5::utils::string::format_epoch(cache_updated);
    }

    auto metadata_expire = repo.get_metadata_expire();
    std::string expire_value;
    if (metadata_expire <= -1) {
        expire_value = "Never";
    } else if (metadata_expire == 0) {
        expire_value = "Instant";
    } else {
        expire_value = fmt::format("{} seconds", metadata_expire);
    }
    add_line("Metadata expire", fmt::format("{} (last: {})", expire_value, last_update));
    add_line("Skip if unavailable", fmt::format("{}", repo.get_skip_if_unavailable()));

    auto repo_file_path = repo.get_repo_file_path();
    if (!repo_file_path.empty()) {
        add_line("Config file", repo_file_path);
    }

    // URLs
    auto group_urls = add_line("URLs", "", nullptr);

    auto base_url = repo.get_baseurl();
    if (!base_url.empty()) {
        add_line("Base URL", base_url, nullptr, group_urls);
    } else {
        auto mirrors = repo.get_mirrors();
        if (!mirrors.empty()) {
            add_line("Base URL", fmt::format("{} ({} more)", mirrors.front(), mirrors.size()), nullptr, group_urls);
        }
    }

    auto metalink = repo.get_metalink();
    auto mirrorlist = repo.get_mirrorlist();
    if (!metalink.empty()) {
        add_line("Metalink", metalink, nullptr, group_urls);
    } else if (!mirrorlist.empty()) {
        add_line("Mirrorlist", mirrorlist, nullptr, group_urls);
    }

    // OpenPGP
    auto group_gpg = add_line("OpenPGP", "", nullptr);

    auto gpg_keys = repo.get_gpgkey();
    if (!gpg_keys.empty()) {
        add_line("Keys", gpg_keys, nullptr, group_gpg);
    }

    add_line("Verify repodata", fmt::format("{}", repo.get_repo_gpgcheck()), nullptr, group_gpg);
    add_line("Verify packages", fmt::format("{}", repo.get_pkg_gpgcheck()), nullptr, group_gpg);

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
    if (enabled) {
        auto group_repodata = add_line("Repodata info", "", nullptr);
        add_line("Available packages", repo.get_available_pkgs(), nullptr, group_repodata);
        add_line("Total packages", repo.get_pkgs(), nullptr, group_repodata);

        std::string size = libdnf5::cli::utils::units::format_size_aligned(static_cast<int64_t>(repo.get_size()));
        add_line("Size", size, nullptr, group_repodata);

        auto content_tags = repo.get_content_tags();
        if (!content_tags.empty()) {
            add_line("Content tags", content_tags, nullptr, group_repodata);
        }

        auto distro_tags = repo.get_distro_tags();
        if (!distro_tags.empty()) {
            add_line("Distro tags", flatten_distro_tags(distro_tags), nullptr, group_repodata);
        }

        add_line("Revision", repo.get_revision(), nullptr, group_repodata);

        add_line("Updated", libdnf5::utils::string::format_epoch(repo.get_max_timestamp()), nullptr, group_repodata);
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


RepoInfo::RepoInfo() : p_impl{new Impl} {}

RepoInfo::~RepoInfo() = default;

void RepoInfo::add_repo(IRepoInfo & repo) {
    p_impl->add_repo(repo);
}

void RepoInfo::print() {
    p_impl->print();
}

void print_repoinfo_json([[maybe_unused]] const std::vector<std::unique_ptr<IRepoInfo>> & repos) {
    json_object * json_repos = json_object_new_array();
    for (const auto & repo : repos) {
        json_object * json_repo = json_object_new_object();
        json_object_object_add(json_repo, "id", json_object_new_string(repo->get_id().c_str()));
        json_object_object_add(json_repo, "name", json_object_new_string(repo->get_name().c_str()));
        json_object_object_add(json_repo, "is_enabled", json_object_new_boolean(repo->is_enabled()));
        json_object_object_add(json_repo, "priority", json_object_new_int(repo->get_priority()));
        json_object_object_add(json_repo, "cost", json_object_new_int(repo->get_cost()));
        json_object_object_add(json_repo, "type", json_object_new_string(repo->get_type().c_str()));

        json_object * json_exclude_pkgs = json_object_new_array();
        for (const auto & pkg : repo->get_excludepkgs()) {
            json_object_array_add(json_exclude_pkgs, json_object_new_string(pkg.c_str()));
        }
        json_object_object_add(json_repo, "exclude_pkgs", json_exclude_pkgs);

        json_object * json_include_pkgs = json_object_new_array();
        for (const auto & pkg : repo->get_includepkgs()) {
            json_object_array_add(json_include_pkgs, json_object_new_string(pkg.c_str()));
        }
        json_object_object_add(json_repo, "include_pkgs", json_include_pkgs);

        json_object_object_add(json_repo, "timestamp", json_object_new_int64(repo->get_timestamp()));
        json_object_object_add(json_repo, "metadata_expire", json_object_new_int(repo->get_metadata_expire()));
        json_object_object_add(
            json_repo, "skip_if_unavailable", json_object_new_boolean(repo->get_skip_if_unavailable()));
        json_object_object_add(json_repo, "repo_file_path", json_object_new_string(repo->get_repo_file_path().c_str()));

        json_object * json_baseurls = json_object_new_array();
        for (const auto & url : repo->get_baseurl()) {
            json_object_array_add(json_baseurls, json_object_new_string(url.c_str()));
        }
        json_object_object_add(json_repo, "base_url", json_baseurls);

        json_object_object_add(json_repo, "metalink", json_object_new_string(repo->get_metalink().c_str()));
        json_object_object_add(json_repo, "mirrorlist", json_object_new_string(repo->get_mirrorlist().c_str()));

        json_object * json_gpg_keys = json_object_new_array();
        for (const auto & key : repo->get_gpgkey()) {
            json_object_array_add(json_gpg_keys, json_object_new_string(key.c_str()));
        }
        json_object_object_add(json_repo, "gpg_key", json_gpg_keys);

        json_object_object_add(json_repo, "repo_gpgcheck", json_object_new_boolean(repo->get_repo_gpgcheck()));
        json_object_object_add(json_repo, "pkg_gpgcheck", json_object_new_boolean(repo->get_pkg_gpgcheck()));
        json_object_object_add(json_repo, "available_pkgs", json_object_new_uint64(repo->get_available_pkgs()));
        json_object_object_add(json_repo, "pkgs", json_object_new_uint64(repo->get_pkgs()));
        json_object_object_add(json_repo, "size", json_object_new_uint64(repo->get_size()));

        json_object * json_content_tags = json_object_new_array();
        for (const auto & tag : repo->get_content_tags()) {
            json_object_array_add(json_content_tags, json_object_new_string(tag.c_str()));
        }
        json_object_object_add(json_repo, "content_tags", json_content_tags);

        json_object * json_distro_tags = json_object_new_array();
        for (const auto & tag : flatten_distro_tags(repo->get_distro_tags())) {
            json_object_array_add(json_distro_tags, json_object_new_string(tag.c_str()));
        }
        json_object_object_add(json_repo, "distro_tags", json_distro_tags);

        json_object_object_add(json_repo, "revision", json_object_new_string(repo->get_revision().c_str()));
        json_object_object_add(json_repo, "max_timestamp", json_object_new_int(repo->get_max_timestamp()));
        json_object_array_add(json_repos, json_repo);
    }
    std::cout << json_object_to_json_string_ext(json_repos, JSON_C_TO_STRING_PRETTY) << std::endl;
    json_object_put(json_repos);
}

}  // namespace libdnf5::cli::output
