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


#include "copr_repo.hpp"

#include "helpers.hpp"
#include "json.hpp"

#include <fnmatch.h>
#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/conf/const.hpp>

#include <filesystem>
#include <fstream>
#include <string>


namespace dnf5 {


std::filesystem::path copr_repo_directory() {
    std::filesystem::path result;
    if (char * dir = getenv("TEST_COPR_CONFIG_DIR")) {
        result = dir;
        return result / "yum.repos.d";
    }
    return COPR_REPO_DIRECTORY;
}


std::string expand_at_in_groupname(const std::string & ownername) {
    if (ownername.starts_with("@"))
        return "group_" + ownername.substr(1);
    return ownername;
}


std::string copr_id_from_repo_id(const std::string & repo_id) {
    /// Convert the repoID to Copr ID (that we can enable or disable using
    /// the 'dnf copr' utility.  Example:
    /// - copr:copr.fedorainfracloud.org:group_copr:copr:suffix
    /// + copr.fedorainfracloud.org/@copr/copr
    /// Return empty string if the conversion isn't possible.

    if (!repo_id.starts_with("copr:"))
        return "";

    // copr.fedorainfracloud.org:group_copr:copr:suffix
    auto copr_id = std::regex_replace(repo_id, std::regex("^copr:"), "");
    // copr.fedorainfracloud.org/group_copr:copr:suffix
    copr_id = std::regex_replace(copr_id, std::regex(":"), "/", std::regex_constants::format_first_only);
    // copr.fedorainfracloud.org/@copr:copr:suffix
    copr_id = std::regex_replace(copr_id, std::regex("/group_"), "/@");
    // copr.fedorainfracloud.org/@copr/copr:suffix
    copr_id = std::regex_replace(copr_id, std::regex(":"), "/", std::regex_constants::format_first_only);
    // drop "multilib" suffix: copr.fedorainfracloud.org/copr/ping:ml
    copr_id = std::regex_replace(copr_id, std::regex(":ml$"), "");
    return copr_id;
}


std::string project_name_from_dirname(const std::string & dirname) {
    return std::regex_replace(dirname, std::regex(":.*"), "");
}


void available_directories_error(
    const std::vector<std::string> & directories, const std::string & owner_name, const std::string & directory_name) {
    std::stringstream message;
    message << libdnf5::utils::sformat(
        _("Directory '{}' not found in '{}' Copr project."),
        owner_name + "/" + directory_name,
        owner_name + "/" + project_name_from_dirname(directory_name));
    message << std::endl;
    bool first = true;

    message << _("You can use one of these available directories:") << std::endl;
    for (const auto & available : directories) {
        if (!first)
            message << std::endl;
        else
            first = false;
        message << " " << owner_name << "/" << available;
    }
    throw std::runtime_error(message.str());
}


void available_chroots_error(
    const std::set<std::string> & chroots, const std::string & chroot, const std::vector<std::string> & fallbacks) {
    std::stringstream message;
    if (chroot != "")
        message << _("Chroot not found in the given Copr project") << " (" << chroot << ").";
    else
        message << _("Unable to detect chroot, specify it explicitly.");

    message << std::endl;

    bool first = true;
    if (fallbacks.size() > 1) {
        message << _("We tried to find these repos without a success:") << std::endl;
        for (const auto & attempted : fallbacks)
            message << " " << attempted << std::endl;
    }

    message << _("You can choose one of the available chroots explicitly:") << std::endl;
    for (const auto & available : chroots) {
        if (!first)
            message << std::endl;
        else
            first = false;
        message << " " << available;
    }
    throw std::runtime_error(message.str());
}

std::string copr_id_to_copr_file(const std::string & repo_id) {
    /// Convert copr ID to a repo filename.
    /// - copr.fedorainfracloud.org/@copr/copr-pull-requests:pr:2545
    /// + _copr:copr.fedorainfracloud.org:group_copr:copr-pull-requests:pr:2545.repo

    std::string output = std::regex_replace(repo_id, std::regex(":ml$"), "");
    output = std::regex_replace(output, std::regex("/"), ":");
    output = std::regex_replace(output, std::regex("@"), "group_");
    return "_copr:" + output + ".repo";
}


std::string get_repo_triplet(
    const std::set<std::string> & available_chroots,
    const std::string & config_name_version,
    const std::string & config_arch,
    std::string & name_version) {
    /// fedora-17 x86_64 => fedora-$releasever-$arch
    /// explicit_chroot  => explicit_chroot (if found)
    /// fedora-eln       => ?
    /// rhel-8           => rhel-$basearch-$arch => epel-$basearch-$arch
    /// centos-8         => rhel-$basearch-$arch => epel-$basearch-$arch

    // all available chroots to enable

    for (const auto & nv : repo_fallbacks(config_name_version)) {
        name_version = nv;
        auto chroot = nv + "-" + config_arch;
        if (available_chroots.contains(chroot)) {
            // do the magic with $releasever and $basearch
            if (nv == "fedora-eln")
                return nv + "-$basearch";
            if (nv.starts_with("fedora-"))
                return "fedora-$releasever-$basearch";

            if (nv.starts_with("opensuse-leap-"))
                return "opensuse-leap-$releasever-$basearch";

            if (nv.starts_with("mageia")) {
                std::string os_version = "$releasever";
                if (nv.ends_with("cauldron")) {
                    os_version = "cauldron";
                }
                return "mageia-" + os_version + "-$basearch";
            }

            return nv + "-$basearch";
        }
    }

    name_version = "";
    return "";
}


static void parse_project_spec(
    const std::string & spec,
    std::string * hubspec = nullptr,
    std::string * ownername = nullptr,
    std::string * project = nullptr,
    std::string * dirname = nullptr) {
    std::smatch match;
    if (!std::regex_match(spec, match, std::regex("^(([^/]+)/)?([^/]+)/([^/]+)$")))
        throw std::runtime_error(_("Can't parse Copr repo spec: ") + spec);
    enum {
        HUBSPEC = 2,
        OWNER = 3,
        DIRNAME = 4,
    };
    if (hubspec)
        *hubspec = match[HUBSPEC];
    if (ownername)
        *ownername = match[OWNER];
    if (dirname)
        *dirname = match[DIRNAME];
    if (project) {
        *project = std::regex_replace(std::string(match[DIRNAME]), std::regex(":.*"), "");
    }
}


CoprRepo::CoprRepo(
    libdnf5::Base & base,
    const std::unique_ptr<CoprConfig> & copr_config,
    const std::string & project_spec,
    const std::string & selected_chroot) {
    this->base = &base;
    enabled = false;
    multilib = false;


    std::string hubspec, project_owner, project_dirname;
    parse_project_spec(project_spec, &hubspec, &project_owner, nullptr, &project_dirname);


    // All available chroots in the selected repo.
    auto config_name_version = copr_config->get_value("main", "name_version");
    auto arch = copr_config->get_value("main", "arch");

    auto url = copr_config->get_repo_url(hubspec, project_owner, project_dirname, config_name_version);
    auto json = std::make_unique<Json>(base, url);

    auto json_repos = json->get_dict_item("repos");
    std::set<std::string> available_chroots;
    for (const auto & name_version : json_repos->keys()) {
        auto arches = json_repos->get_dict_item(name_version)->get_dict_item("arch");
        for (const auto & arch_found : arches->keys()) {
            auto chroot = name_version + "-" + arch_found;
            available_chroots.insert(chroot);
        }
    }

    auto project_name = project_name_from_dirname(project_dirname);

    const auto & dirnames = json->get_dict_item("directories")->keys();
    if (!std::any_of(dirnames.begin(), dirnames.end(), [&](const auto & d) { return d == project_dirname; }))
        available_directories_error(dirnames, project_owner, project_dirname);

    std::string baseurl_chroot = "";
    std::string json_selector;
    if (selected_chroot != "") {
        // We do not expand $basearch and $releasever here because user
        // explicitly asked for a particular some chroot (which might
        // intentionally be a different distro or a cross-arch chroot).
        baseurl_chroot = selected_chroot;
        if (!available_chroots.contains(selected_chroot))
            available_chroots_error(available_chroots, selected_chroot);
        // Chroot can look like: fedora-rawhide-x86_64
        // Split on second/last "-" to: "fedora-rawhide" and "x86_64"
        size_t second_dash_pos = selected_chroot.find_last_of("-");
        json_selector = selected_chroot.substr(0, second_dash_pos);
        arch = selected_chroot.substr(second_dash_pos + 1);
    } else {
        baseurl_chroot = get_repo_triplet(available_chroots, config_name_version, arch, json_selector);
        if (baseurl_chroot.empty()) {
            auto detected_chroot = config_name_version + "-" + arch;
            auto fallbacks = repo_fallbacks(config_name_version);
            available_chroots_error(available_chroots, detected_chroot, fallbacks);
        }
    }

    auto owner = expand_at_in_groupname(project_owner);
    std::string repo_id = "copr:" + copr_config->get_hub_hostname(hubspec) + ":" + owner + ":" + project_dirname;
    set_id_from_repo_id(repo_id);

    std::string results_url = json->get_dict_item("results_url")->string();
    std::string baseurl = results_url + "/" + project_owner + "/" + project_dirname + "/" + baseurl_chroot + "/";
    std::string name = "Copr repo for " + project_dirname + " owned by " + project_owner;

    std::string gpgkey = results_url + "/" + project_owner + "/" + project_name + "/" + "pubkey.gpg";
    if (json->has_key("pubkey_url")) {
        gpgkey = json->get_dict_item("pubkey_url")->string();
    }

    auto main_repo_json = json_repos->get_dict_item(json_selector)->get_dict_item("arch")->get_dict_item(arch);
    auto main_repo = CoprRepoPart(repo_id, name, true, baseurl, gpgkey);
    main_repo.update_from_json_opts(main_repo_json);
    add_repo_part(main_repo);

    // multilib repositories
    if (selected_chroot.empty() && main_repo_json->has_key("multilib")) {
        auto mljson = main_repo_json->get_dict_item("multilib");
        int suffix = 0;
        for (auto key : mljson->keys()) {
            std::string ml_suffix = ":ml";
            if (suffix) {
                ml_suffix += std::to_string(suffix);
            }
            std::string multilib_id = repo_id + ml_suffix;
            auto multilib_chroot = std::regex_replace(baseurl_chroot, std::regex("\\$basearch"), key);
            baseurl = results_url + "/" + project_owner + "/" + project_dirname + "/" + multilib_chroot + "/";
            auto ml_repo = CoprRepoPart(multilib_id, name + " (" + key + ")", true, baseurl, gpgkey);
            ml_repo.update_from_json_opts(main_repo_json);
            ml_repo.update_from_json_opts(mljson->get_dict_item(key));
            add_repo_part(ml_repo);
            suffix += 1;
        }
    }

    auto deps = json->get_dict_item("dependencies");

    // external dependencies
    for (size_t i = 0; i < deps->array_length(); i++) {
        auto dep = deps->get_array_item(i);

        auto type = dep->get_dict_item("type")->string();
        if (type == "copr") {
            auto repo = CoprRepoPart(dep, results_url, baseurl_chroot);
            add_repo_part(repo);
        } else if (type == "external_baseurl") {
            auto repo = CoprRepoPart(dep, baseurl_chroot);
            add_repo_part(repo);
        }
    }
}


std::filesystem::path CoprRepo::file_path() const {
    std::filesystem::path path = copr_repo_directory();
    path /= copr_id_to_copr_file(id);
    return path;
}


void CoprRepo::remove() const {
    std::string path = file_path().native();
    if (-1 == unlink(path.c_str())) {
        std::string msg = libdnf5::utils::sformat(_("Can't remove the {} repo file"), path);
        throw std::runtime_error(msg);
    }
    std::cout << libdnf5::utils::sformat(_("Repo file {} successfully removed"), path) << std::endl;
}


bool CoprRepo::matches_repospec(const std::string & repo_spec) {
    // auto copr_id = repo_id_from_project_spec(repo_spec);
    // return id == copr_id;
    return repo_spec == "";
}

void CoprRepo::disable() {
    for (auto & p : repositories)
        p.second.disable();
}

void CoprRepo::save() {
    std::filesystem::path path = file_path();
    std::ofstream repofile;
    repofile.open(path);
    repofile << *this;
    repofile.close();

    std::filesystem::permissions(
        path,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write | std::filesystem::perms::group_read |
            std::filesystem::perms::others_read,
        std::filesystem::perm_options::add);

    remove_old_repo();
}


bool CoprRepoPart::is_multilib() {
    return (fnmatch("copr:*:*:*:ml", id.c_str(), 0) == 0);
}


static std::string nth_delimited_item(const std::string & string, size_t n, char separator = ':') {
    std::stringstream ss(string);
    std::string token;
    size_t i = 0;
    while (getline(ss, token, separator)) {
        if (i >= n)
            return token;
        i++;
    }
    std::string msg = libdnf5::utils::sformat(_("Can't find item {} in {}"), n, string);
    throw std::runtime_error(msg);
}

std::string CoprRepo::get_ownername() {
    return nth_delimited_item(id, 1, '/');
}

std::string CoprRepo::get_projectname() {
    return nth_delimited_item(id, 2, '/');
}


void CoprRepo::save_interactive() {
    std::cerr << COPR_THIRD_PARTY_WARNING;
    if (!libdnf5::cli::utils::userconfirm::userconfirm(base->get_config()))
        return;

    if (has_external_deps()) {
        std::stringstream the_list;
        int i = 0;
        for (const auto & p : repositories) {
            auto & repo = p.second;
            if (!repo.is_external())
                continue;
            if (i)
                the_list << std::endl;
            i++;
            the_list << std::setiosflags(std::ios::right);
            the_list << std::setw(3) << i;
            the_list << std::setiosflags(std::ios::left);
            the_list << ". [" << repo.get_id() << "]" << std::endl;
            the_list << "     baseurl=" << repo.get_baseurl() << std::endl;
        }

        std::cerr << std::endl;
        std::cerr << libdnf5::utils::sformat(COPR_EXTERNAL_DEPS_WARNING, the_list.str());
        std::cerr << std::endl;
        if (!libdnf5::cli::utils::userconfirm::userconfirm(base->get_config())) {
            for (auto & p : repositories) {
                auto & repo = p.second;
                if (!repo.is_external())
                    continue;
                repo.disable();
            }
        }
    }  // if has_external_deps()

    save();
}

void CoprRepo::remove_old_repo() {
    std::filesystem::path path = copr_repo_directory();
    path /= "_copr_" + get_ownername() + "-" + get_projectname() + ".repo";

    if (std::filesystem::exists(path)) {
        std::cerr << libdnf5::utils::sformat(_("Removing old config file '{}'"), path.native()) << std::endl;

        if (unlink(path.native().c_str()))
            throw std::runtime_error(_("Can't remove"));
    }
}

// bool partly_enabled = false
void CoprRepo::add_dnf_repo(libdnf5::repo::RepoWeakPtr dnfRepo) {
    set_id_from_repo_id(dnfRepo->get_id());
    auto cp = CoprRepoPart(dnfRepo);
    this->enabled |= cp.is_enabled();
    if (repo_file.empty())
        repo_file = dnfRepo->get_repo_file_path();
    if (cp.is_multilib())
        this->multilib = true;
    add_repo_part(cp);
}

void CoprRepo::set_id_from_repo_id(const std::string & dnfRepoID) {
    // copr:copr.fedorainfracloud.org:group_codescan:csutils
    // copr.fedorainfracloud.org/@codescan/csutils
    if (id != "")
        return;
    auto copr_id = copr_id_from_repo_id(dnfRepoID);
    if (copr_id != "")
        id = copr_id;
}

void CoprRepo::load_raw_values(libdnf5::ConfigParser & parser) {
    for (auto & pair : repositories) {
        pair.second.load_raw_values(parser, repo_file);
    }
}

void CoprRepo::add_repo_part(const CoprRepoPart & rp) {
    repositories[rp.get_id()] = rp;
}

void CoprRepoPart::load_raw_values(libdnf5::ConfigParser & parser, const std::filesystem::path & path) {
    parser.read(path.native());
    if (parser.has_option(id, "baseurl"))
        baseurl = parser.get_value(id, "baseurl");
    if (parser.has_option(id, "gpgkey"))
        gpgkey = parser.get_value(id, "gpgkey");
}

std::ostream & operator<<(std::ostream & os, const class CoprRepo & copr_repo) {
    bool first = true;
    for (const auto & p : copr_repo.repositories) {
        if (!first)
            os << std::endl;
        first = false;
        os << p.second;
    }
    return os;
}


static const std::string guess_project_name_from_old_repofile_name(const std::string & repo_file_basename) {
    auto output = std::regex_replace(repo_file_basename, std::regex("^_copr_"), "");
    output = std::regex_replace(output, std::regex(".repo$"), "");
    return std::regex_replace(output, std::regex("-"), "/", std::regex_constants::format_first_only);
}


static void warn_old_copr_repofile(const std::filesystem::path & repofile) {
    /// This is a very old Copr repository format we want to warn
    /// about, as it used to be defined here:
    /// https://github.com/rpm-software-management/dnf-plugins-core/blob/9905d15bfcb2df00e4a0f9ad5f40453111f8f96b/plugins/copr.py#L182-L183
    /// Even DNF4 copr.py was fixed for a long time, but e.g. the
    /// fedora-workstation-repositories.rpm still provides an old formatted
    /// repoffile.
    auto base = repofile.filename().native();
    if (!base.starts_with("_copr_"))
        return;
    warning(
        _("Copr repofile '{}' has an old format, "
          "fix by re-enabling it $ {} copr enable {}"),
        repofile.c_str(),
        COPR_DNF5_COMMAND,
        guess_project_name_from_old_repofile_name(base).c_str());
}


static bool is_copr_repofile(const std::filesystem::path & repofile) {
    return repofile.filename().native().starts_with("_copr:");
}


static std::string repo_id_to_copr_file(const std::string & repo_id) {
    /// Convert repoID (generated by Copr Frontend) to a repo filename.  Note we
    /// keep the group_ prefix for group ownernames.
    /// - copr:copr.fedorainfracloud.org:group_copr:copr-pull-requests:pr:2545
    /// + _copr:copr.fedorainfracloud.org:group_copr:copr-pull-requests:pr:2545.repo

    auto fixed_repo_id = std::regex_replace(repo_id, std::regex(":ml$"), "");
    return "_" + fixed_repo_id + ".repo";
}


static void warn_weird_copr_repo(libdnf5::repo::RepoWeakPtr dnfRepo) {
    /// Throw a warning for every copr-repofile-related repo ID which doesn't
    /// follow the pre-defined format.

    std::string repo_id = dnfRepo->get_id();
    if (repo_id.starts_with("coprdep:"))
        return;

    auto repo_file_path = dnfRepo->get_repo_file_path().c_str();
    if (!repo_id.starts_with("copr:")) {
        warning(_("Repo id '{}' in '{}' is suspicious"), repo_id.c_str(), repo_file_path);
        return;
    }

    // Check that the repo filename matches the repo ID
    std::filesystem::path path = dnfRepo->get_repo_file_path();
    if (repo_id_to_copr_file(repo_id) != path.filename().native()) {
        warning(_("Repo id '{}' doesn't belong to '{}' repofile"), repo_id.c_str(), repo_file_path);
    }
}


void installed_copr_repositories(libdnf5::Base & base, CoprRepoCallback cb) {
    std::map<std::string, CoprRepo> copr_repos;

    libdnf5::repo::RepoQuery query(base);
    for (const auto & repo : query.get_data()) {
        auto repo_file_path = repo->get_repo_file_path();

        warn_old_copr_repofile(repo_file_path);
        if (!is_copr_repofile(repo_file_path))
            continue;

        warn_weird_copr_repo(repo);
        auto [it, inserted] = copr_repos.emplace(repo_file_path, CoprRepo(base));
        it->second.add_dnf_repo(repo);
    }

    for (auto & [key, copr_repo] : copr_repos)
        cb(copr_repo);
}


static std::string repo_id_from_project_spec(libdnf5::Base & base, const std::string & project_spec_string) {
    std::string hubspec, ownername, dirname;
    parse_project_spec(project_spec_string, &hubspec, &ownername, nullptr, &dirname);

    auto config = std::make_unique<CoprConfig>(base);
    auto hub_host = config->get_hub_hostname(hubspec);
    return hub_host + "/" + ownername + "/" + dirname;
}


class RepoDisableCB : public CoprRepoCallback {
private:
    std::string id;
    libdnf5::ConfigParser & parser;

public:
    int count = 0;
    explicit RepoDisableCB(const std::string & id, libdnf5::ConfigParser & cp) : id(id), parser(cp) {}
    dnf5::CoprRepoCallback disable = [&](dnf5::CoprRepo & cr) {
        if (id == cr.get_id()) {
            cr.load_raw_values(parser);
            cr.disable();
            cr.save();
            count++;
            std::cout << libdnf5::utils::sformat(
                             _("Copr repository '{}' in '{}' disabled."), cr.get_id(), cr.get_repo_file_path().native())
                      << std::endl;
        }
    };
};


void copr_repo_disable(libdnf5::Base & base, const std::string & project_spec_string) {
    libdnf5::ConfigParser parser;
    auto repo_id = repo_id_from_project_spec(base, project_spec_string);
    auto disable = RepoDisableCB(repo_id, parser);
    installed_copr_repositories(base, disable.disable);

    if (disable.count == 0)
        throw std::runtime_error(libdnf5::utils::sformat(_("Repository '{}' not found on this system"), repo_id));
}


class RepoRemoveCB : public CoprRepoCallback {
private:
    std::string id;

public:
    int count = 0;
    explicit RepoRemoveCB(const std::string & id) : id(id) {}
    dnf5::CoprRepoCallback remove = [&](dnf5::CoprRepo & cr) {
        if (id == cr.get_id()) {
            cr.remove();
            count++;
        }
    };
};

void copr_repo_remove(libdnf5::Base & base, const std::string & project_spec_string) {
    auto repo_id = repo_id_from_project_spec(base, project_spec_string);
    auto remover = RepoRemoveCB(repo_id);
    installed_copr_repositories(base, remover.remove);

    if (remover.count == 0)
        throw std::runtime_error(libdnf5::utils::sformat(_("Repository '{}' not found on this system"), repo_id));
}


}  // namespace dnf5
