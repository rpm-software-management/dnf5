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

#include "repo_info.hpp"

#include <libdnf5-cli/output/interfaces/repo.hpp>
#include <libdnf5-cli/output/repo_info.hpp>
#include <libdnf5/rpm/package_query.hpp>

#include <iostream>

namespace dnf5 {

class RepoInfoWrapper : public libdnf5::cli::output::IRepoInfo {
public:
    RepoInfoWrapper(libdnf5::repo::Repo & repo, uint64_t size, uint64_t pkgs, uint64_t available_pkgs)
        : repo(&repo),
          size(size),
          pkgs(pkgs),
          available_pkgs(available_pkgs) {}

    std::string get_id() const override { return repo->get_id(); }
    std::string get_name() const override { return repo->get_name(); }
    std::string get_type() const override { return libdnf5::repo::Repo::type_to_string(repo->get_type()); }
    bool is_enabled() const override { return repo->is_enabled(); }
    int get_priority() const override { return repo->get_config().get_priority_option().get_value(); }
    int get_cost() const override { return repo->get_config().get_cost_option().get_value(); }
    std::vector<std::string> get_baseurl() const override {
        return repo->get_config().get_baseurl_option().get_value();
    }
    std::string get_metalink() const override {
        auto & option = repo->get_config().get_metalink_option();
        if (option.empty()) {
            return "";
        } else {
            return option.get_value();
        }
    }
    std::string get_mirrorlist() const override {
        auto & option = repo->get_config().get_mirrorlist_option();
        if (option.empty()) {
            return "";
        } else {
            return option.get_value();
        }
    }
    int get_metadata_expire() const override { return repo->get_config().get_metadata_expire_option().get_value(); }
    std::vector<std::string> get_excludepkgs() const override {
        return repo->get_config().get_excludepkgs_option().get_value();
    }
    std::vector<std::string> get_includepkgs() const override {
        return repo->get_config().get_includepkgs_option().get_value();
        ;
    }
    bool get_skip_if_unavailable() const override {
        return repo->get_config().get_skip_if_unavailable_option().get_value();
    }
    std::vector<std::string> get_gpgkey() const override { return repo->get_config().get_gpgkey_option().get_value(); }
    bool get_gpgcheck() const override { return repo->get_config().get_gpgcheck_option().get_value(); }
    bool get_repo_gpgcheck() const override { return repo->get_config().get_repo_gpgcheck_option().get_value(); }
    std::string get_repo_file_path() const override { return repo->get_repo_file_path(); }
    std::string get_revision() const override { return repo->get_revision(); }
    std::vector<std::string> get_content_tags() const override { return repo->get_content_tags(); }
    std::vector<std::pair<std::string, std::string>> get_distro_tags() const override {
        return repo->get_distro_tags();
    }
    int64_t get_timestamp() const override { return repo->get_timestamp(); }
    int get_max_timestamp() const override { return repo->get_max_timestamp(); }
    uint64_t get_size() const override { return size; }
    uint64_t get_pkgs() const override { return pkgs; }
    uint64_t get_available_pkgs() const override { return available_pkgs; }
    std::vector<std::string> get_mirrors() const override { return repo->get_mirrors(); }

private:
    libdnf5::repo::Repo * repo;
    uint64_t size;
    uint64_t pkgs;
    uint64_t available_pkgs;
};

void RepoInfoCommand::configure() {
    auto & context = get_context();
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void RepoInfoCommand::print(const libdnf5::repo::RepoQuery & query, [[maybe_unused]] bool with_status) {
    // sort the query to always get the same results
    std::vector<libdnf5::repo::RepoWeakPtr> repos;
    for (auto & repo : query) {
        repos.emplace_back(repo);
    }
    std::sort(repos.begin(), repos.end(), [](const auto & l, const auto & r) { return l->get_id() < r->get_id(); });

    for (auto & repo : repos) {
        libdnf5::rpm::PackageQuery pkgs(get_context().base, libdnf5::sack::ExcludeFlags::IGNORE_EXCLUDES);
        pkgs.filter_repo_id({repo->get_id()});

        libdnf5::rpm::PackageQuery available_pkgs(get_context().base);
        available_pkgs.filter_repo_id({repo->get_id()});

        uint64_t repo_size = 0;
        for (const auto & pkg : pkgs) {
            repo_size += pkg.get_download_size();
        }

        libdnf5::cli::output::RepoInfo repo_info_table;
        RepoInfoWrapper repo_wrapper(*repo, repo_size, pkgs.size(), available_pkgs.size());
        repo_info_table.add_repo(repo_wrapper);
        repo_info_table.print();
        std::cout << std::endl;
    }
}

}  // namespace dnf5
