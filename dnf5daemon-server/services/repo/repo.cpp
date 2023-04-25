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

#include "repo.hpp"

#include "dbus.hpp"
#include "utils.hpp"

#include <fmt/format.h>
#include <libdnf/repo/repo.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <chrono>
#include <iostream>
#include <string>

namespace {

// repository attributes available to be retrieved
// based on `dnf repolist` command
enum class RepoAttribute {
    // from repo configuration
    id,
    name,
    enabled,
    baseurl,
    metalink,
    mirrorlist,
    metadata_expire,
    excludepkgs,
    includepkgs,
    repofile,

    // require metadata loading
    revision,
    content_tags,
    distro_tags,
    updated,
    pkgs,
    available_pkgs,  // number of not excluded packages
    size,

    proxy,
    proxy_username,
    proxy_password,
};

std::vector<RepoAttribute> metadata_required_attributes{
    RepoAttribute::revision,
    RepoAttribute::content_tags,
    RepoAttribute::distro_tags,
    RepoAttribute::updated,
    RepoAttribute::pkgs,
    RepoAttribute::available_pkgs,
    RepoAttribute::size};

// map string package attribute name to actual attribute
const static std::map<std::string, RepoAttribute> repo_attributes{
    {"id", RepoAttribute::id},
    {"name", RepoAttribute::name},
    {"enabled", RepoAttribute::enabled},
    {"baseurl", RepoAttribute::baseurl},
    {"metalink", RepoAttribute::metalink},
    {"mirrorlist", RepoAttribute::mirrorlist},
    {"metadata_expire", RepoAttribute::metadata_expire},
    {"excludepkgs", RepoAttribute::excludepkgs},
    {"includepkgs", RepoAttribute::includepkgs},
    {"repofile", RepoAttribute::repofile},
    {"revision", RepoAttribute::revision},
    {"content_tags", RepoAttribute::content_tags},
    {"distro_tags", RepoAttribute::distro_tags},
    {"updated", RepoAttribute::updated},
    {"pkgs", RepoAttribute::pkgs},
    {"available_pkgs", RepoAttribute::available_pkgs},
    {"size", RepoAttribute::size},

    {"proxy", RepoAttribute::proxy},
    {"proxy_username", RepoAttribute::proxy_username},
    {"proxy_password", RepoAttribute::proxy_password},
};

// converts Repo object to dbus map
dnfdaemon::KeyValueMap repo_to_map(
    libdnf::Base & base,
    const libdnf::WeakPtr<libdnf::repo::Repo, false> libdnf_repo,
    std::vector<std::string> & attributes) {
    dnfdaemon::KeyValueMap dbus_repo;
    // attributes required by client
    for (auto & attr : attributes) {
        switch (repo_attributes.at(attr)) {
            case RepoAttribute::id:
                dbus_repo.emplace(attr, libdnf_repo->get_id());
                break;
            case RepoAttribute::name:
                dbus_repo.emplace(attr, libdnf_repo->get_config().get_name_option().get_value());
                break;
            case RepoAttribute::enabled:
                dbus_repo.emplace(attr, libdnf_repo->is_enabled());
                break;
            case RepoAttribute::size: {
                uint64_t size = 0;
                libdnf::rpm::PackageQuery query(base);
                std::vector<std::string> reponames = {libdnf_repo->get_id()};
                query.filter_repo_id(reponames);
                for (auto pkg : query) {
                    size += pkg.get_package_size();
                }
                dbus_repo.emplace(attr, size);
            } break;
            case RepoAttribute::revision:
                dbus_repo.emplace(attr, libdnf_repo->get_revision());
                break;
            case RepoAttribute::content_tags:
                dbus_repo.emplace(attr, libdnf_repo->get_content_tags());
                break;
            case RepoAttribute::distro_tags: {
                // sdbus::Variant cannot accomodate a std::pair
                std::vector<std::string> distro_tags{};
                for (auto & dt : libdnf_repo->get_distro_tags()) {
                    distro_tags.emplace_back(dt.first);
                    distro_tags.emplace_back(dt.second);
                }
                dbus_repo.emplace(attr, distro_tags);
            } break;
            case RepoAttribute::updated:
                dbus_repo.emplace(attr, libdnf_repo->get_max_timestamp());
                break;
            case RepoAttribute::pkgs: {
                libdnf::rpm::PackageQuery query(base, libdnf::rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
                std::vector<std::string> reponames = {libdnf_repo->get_id()};
                query.filter_repo_id(reponames);
                dbus_repo.emplace(attr, query.size());
            } break;
            case RepoAttribute::available_pkgs: {
                libdnf::rpm::PackageQuery query(base);
                std::vector<std::string> reponames = {libdnf_repo->get_id()};
                query.filter_repo_id(reponames);
                dbus_repo.emplace(attr, query.size());
            } break;
            case RepoAttribute::metalink: {
                auto opt = libdnf_repo->get_config().get_metalink_option();
                dbus_repo.emplace(attr, (opt.empty() || opt.get_value().empty()) ? "" : opt.get_value());
            } break;
            case RepoAttribute::mirrorlist: {
                auto opt = libdnf_repo->get_config().get_mirrorlist_option();
                dbus_repo.emplace(attr, (opt.empty() || opt.get_value().empty()) ? "" : opt.get_value());
            } break;
            case RepoAttribute::baseurl:
                dbus_repo.emplace(attr, libdnf_repo->get_config().get_baseurl_option().get_value());
                break;
            case RepoAttribute::metadata_expire:
                dbus_repo.emplace(attr, libdnf_repo->get_config().get_metadata_expire_option().get_value());
                break;
            case RepoAttribute::excludepkgs:
                dbus_repo.emplace(attr, libdnf_repo->get_config().get_excludepkgs_option().get_value());
                break;
            case RepoAttribute::includepkgs:
                dbus_repo.emplace(attr, libdnf_repo->get_config().get_includepkgs_option().get_value());
                break;
            case RepoAttribute::repofile:
                dbus_repo.emplace(attr, libdnf_repo->get_repo_file_path());
                break;

            // proxy
            case RepoAttribute::proxy:
                dbus_repo.emplace(attr, libdnf_repo->get_config().get_proxy_option().get_value());
                break;
            case RepoAttribute::proxy_username:
                //                dbus_repo.emplace(attr, libdnf_repo->get_config().get_proxy_username_option().get_value());
                dbus_repo.emplace(attr, "user foo");
                break;
            case RepoAttribute::proxy_password:
                dbus_repo.emplace(attr, libdnf_repo->get_config().get_proxy_password_option().get_value());
                break;
        }
    }
    return dbus_repo;
}

bool keyval_repo_compare(const dnfdaemon::KeyValueMap & first, const dnfdaemon::KeyValueMap & second) {
    return key_value_map_get<std::string>(first, "id") < key_value_map_get<std::string>(second, "id");
}

}  // namespace

void Repo::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_REPO, "list", "a{sv}", "aa{sv}", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Repo::list, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_REPO, "confirm_key", "sb", "", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &Repo::confirm_key, call);
        });
}

sdbus::MethodReply Repo::confirm_key(sdbus::MethodCall & call) {
    std::string key_id;
    bool confirmed;
    call >> key_id >> confirmed;
    if (confirmed) {
        if (!session.check_authorization(dnfdaemon::POLKIT_CONFIRM_KEY_IMPORT, call.getSender())) {
            session.confirm_key(key_id, false);
            throw std::runtime_error("Not authorized");
        }
    }
    session.confirm_key(key_id, confirmed);
    return call.createReply();
}

sdbus::MethodReply Repo::list(sdbus::MethodCall & call) {
    dnfdaemon::KeyValueMap options;
    call >> options;
    const std::vector<std::string> empty_list{};

    // read options from dbus call
    std::string enable_disable = key_value_map_get<std::string>(options, "enable_disable", "enabled");
    std::vector<std::string> patterns = key_value_map_get<std::vector<std::string>>(options, "patterns", empty_list);
    // check demanded attributes
    std::vector<std::string> repo_attrs =
        key_value_map_get<std::vector<std::string>>(options, "repo_attrs", empty_list);
    bool fill_sack_needed = false;
    for (auto & attr_str : repo_attrs) {
        if (repo_attributes.count(attr_str) == 0) {
            throw std::runtime_error(fmt::format("Repo attribute '{}' not supported", attr_str));
        }
        if (!fill_sack_needed) {
            fill_sack_needed = std::find(
                                   metadata_required_attributes.begin(),
                                   metadata_required_attributes.end(),
                                   repo_attributes.at(attr_str)) != metadata_required_attributes.end();
        }
    }
    // always return repoid
    repo_attrs.push_back("id");
    if (fill_sack_needed) {
        session.fill_sack();
    }

    // prepare repository query filtered by options
    auto base = session.get_base();
    libdnf::repo::RepoQuery repos_query(*base);

    if (enable_disable == "enabled") {
        repos_query.filter_enabled(true);
    } else if (enable_disable == "disabled") {
        repos_query.filter_enabled(false);
    }

    repos_query.filter_type(libdnf::repo::Repo::Type::AVAILABLE);

    if (patterns.size() > 0) {
        auto query_names = repos_query;
        query_names.filter_name(patterns, libdnf::sack::QueryCmp::IGLOB);
        repos_query.filter_id(patterns, libdnf::sack::QueryCmp::IGLOB);
        repos_query |= query_names;
    }

    // create reply from the query
    dnfdaemon::KeyValueMapList out_repositories;

    for (auto & repo : repos_query) {
        out_repositories.push_back(repo_to_map(*base, repo, repo_attrs));
    }

    std::sort(out_repositories.begin(), out_repositories.end(), keyval_repo_compare);
    auto reply = call.createReply();
    reply << out_repositories;
    return reply;
}
