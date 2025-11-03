// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5_COMMANDS_COPR_REPO_HPP
#define DNF5_COMMANDS_COPR_REPO_HPP

#include "copr_config.hpp"
#include "copr_constants.hpp"
#include "helpers.hpp"
#include "json.hpp"

#include <libdnf5/base/base.hpp>

#include <filesystem>
#include <regex>


namespace dnf5 {


void available_chroots_error(
    const std::set<std::string> & chroots,
    const std::string & chroot = "",
    const std::vector<std::string> & fallbacks = std::vector<std::string>());


class CoprRepoPart {
public:
    CoprRepoPart() {};
    explicit CoprRepoPart(libdnf5::repo::RepoWeakPtr dnfRepo) {
        /// Copr enable/disable simply always generates the repofile from
        /// scratch ('enable' from the json provided by Copr Frontend, 'disable'
        /// loads the data from repofiles).
        /// Some of the values loaded, when loaded from config, have expanded
        /// DNF Vars.  Therefore for 'disable' operation we have to load the raw
        /// values first using load_raw_values().
        auto base = dnfRepo->get_base();
        auto & config = dnfRepo->get_config();
        id = dnfRepo->get_id();
        name = config.get_name_option().get_value_string();
        enabled = dnfRepo->is_enabled();
        priority = dnfRepo->get_priority();
        cost = dnfRepo->get_cost();
        module_hotfixes = config.get_module_hotfixes_option().get_value();
    };

    explicit CoprRepoPart(
        const std::string & id,
        const std::string & name,
        bool enabled,
        const std::string & baseurl,
        const std::string & gpgkey)
        : id(id),
          name(name),
          enabled(enabled),
          baseurl(baseurl),
          gpgkey(gpgkey) {}

    explicit CoprRepoPart(const std::unique_ptr<Json> & json_dep, const std::string & chroot) {
        update_from_json_opts(json_dep);
        auto data = json_dep->get_dict_item("data");
        auto pattern = data->get_dict_item("pattern")->string();
        auto distname = std::regex_replace(chroot, std::regex("-[^-]+-[^-]+$"), "");
        baseurl = std::regex_replace(pattern, std::regex("\\$chroot"), chroot);
        baseurl = std::regex_replace(baseurl, std::regex("\\$distname"), distname);
    }

    explicit CoprRepoPart(
        const std::unique_ptr<Json> & json_dep, const std::string & results_url, const std::string & chroot) {
        update_from_json_opts(json_dep);
        auto info = json_dep->get_dict_item("data");
        auto owner = info->get_dict_item("owner")->string();
        auto project = info->get_dict_item("projectname")->string();
        set_copr_pub_key(results_url, owner, project);
        set_copr_baseurl(results_url, owner, project, chroot);
    }

    friend std::ostream & operator<<(std::ostream & os, const class CoprRepoPart & repo) {
        os << "[" << repo.id << "]" << std::endl;
        os << "name=" << repo.name << std::endl;
        os << "baseurl=" << repo.baseurl << std::endl;
        os << "type=rpm-md" << std::endl;
        os << "skip_if_unavailable=True" << std::endl;
        os << "gpgcheck=" << (repo.gpgkey.empty() ? 0 : 1) << std::endl;
        if (repo.gpgkey != "")
            os << "gpgkey=" << repo.gpgkey << std::endl;
        os << "repo_gpgcheck=0" << std::endl;
        if (repo.cost && repo.cost != 1000)
            os << "cost=" << repo.cost << std::endl;
        os << "enabled=" << (repo.enabled ? "1" : "0") << std::endl;
        os << "enabled_metadata=1" << std::endl;
        if (repo.priority != 99)
            os << "priority=" << repo.priority << std::endl;
        if (repo.module_hotfixes)
            os << "module_hotfixes=1" << std::endl;
        return os;
    }

    void update_from_json_opts(const std::unique_ptr<Json> & json) {
        /// Copr enable/disable simply always generates the repofile from
        /// scratch.  This method is more about the 'enable' part.
        /// WARNING: whenever you touch this method, you likely want to update
        /// also the CoprRepoPart::load_raw_values() or
        /// CoprRepoPart::CoprRepoPart(dnfRepo) which are more about the
        /// 'disable' part.

        // Copr Frontend API-responds to us with this particular CoprRepoPart
        // included, so it _is_ supposed to be enabled (otherwise it would not
        // be listed).
        enabled = true;

        if (!json->has_key("opts"))
            return;
        auto opts = json->get_dict_item("opts");
        for (const auto & key : opts->keys()) {
            auto val = opts->get_dict_item(key);
            if (key == "cost")
                cost = std::stoi(val->string());
            else if (key == "priority")
                priority = std::stoi(val->string());
            else if (key == "module_hotfixes")
                module_hotfixes = val->boolean();
            else if (key == "id")
                id = val->string();
            else if (key == "name")
                name = val->string();
        }
    }
    void load_raw_values(libdnf5::ConfigParser & parser, const std::filesystem::path & path);

    void set_copr_pub_key(const std::string & results_url, const std::string & owner, const std::string & projectname) {
        gpgkey = results_url + "/" + owner + "/" + projectname + "/pubkey.gpg";
    }

    void set_copr_baseurl(
        const std::string & results_url,
        const std::string & owner,
        const std::string & dirname,
        const std::string & chroot) {
        baseurl = results_url + "/" + owner + "/" + dirname + "/" + chroot + "/";
    }

    bool is_external() const { return id.starts_with("coprdep:"); };
    bool is_enabled() { return enabled; };
    bool is_multilib();
    std::string get_id() const { return id; };
    std::string get_baseurl() const { return baseurl; };
    void disable() { enabled = false; };

private:
    std::string id;
    std::string name;
    bool enabled;
    std::string baseurl;
    std::string gpgkey;
    int priority = 99;
    int cost = 0;
    bool module_hotfixes = false;
};


class CoprRepo {
public:
    explicit CoprRepo(libdnf5::Base & base) : base(&base) {}

    explicit CoprRepo(
        libdnf5::Base & base,
        const std::unique_ptr<CoprConfig> & copr_config,
        const std::string & project_spec,
        const std::string & selected_chroot);

    std::filesystem::path file_path() const;
    void save();
    void remove() const;
    void disable();
    void save_interactive();

    friend std::ostream & operator<<(std::ostream & os, const class CoprRepo & copr_repo);

    // bool partly_enabled = false
    void add_dnf_repo(libdnf5::repo::RepoWeakPtr dnfRepo);

    bool is_multilib() const { return multilib; };
    bool is_enabled() const { return enabled; };
    std::string get_id() const { return id; };

    std::string get_ownername();
    std::string get_projectname();
    std::filesystem::path get_repo_file_path() { return repo_file; };

    bool has_external_deps() const {
        const auto & r = repositories;
        return std::any_of(r.begin(), r.end(), [&](const auto & p) { return p.second.is_external(); });
    }

    void load_raw_values(libdnf5::ConfigParser & parser);

    void add_repo_part(const CoprRepoPart & rp);

    bool matches_repospec(const std::string & repo_spec);

private:
    libdnf5::Base * base{nullptr};
    std::string id;         /// the id, groups are like '@GROUPNAME'
    std::string repo_file;  /// full path
    std::map<std::string, CoprRepoPart> repositories;
    bool enabled = false;
    bool multilib = false;

    void set_id_from_repo_id(const std::string & dnfRepoID);
    void remove_old_repo();
};

using CoprRepoCallback = std::function<void(CoprRepo &)>;
void installed_copr_repositories(libdnf5::Base & base, CoprRepoCallback cb);
std::filesystem::path copr_repo_directory(libdnf5::Base * base);

void copr_repo_disable(libdnf5::Base & base, const std::string & repo_spec);
void copr_repo_remove(libdnf5::Base & base, const std::string & repo_spec);

}  // namespace dnf5

#endif  // DNF5_COMMANDS_COPR_REPO_HPP
