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


#include "obs_repo.hpp"
#include "download_file.hpp"
#include "helpers.hpp"

#include <fnmatch.h>
#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/utils/fs/temp.hpp>
#include <libdnf5/repo/file_downloader_errors.hpp>

#include <filesystem>
#include <fstream>
#include <string>


namespace dnf5 {


std::filesystem::path obs_repo_directory(libdnf5::Base * base) {
    std::filesystem::path result;

    std::filesystem::path installroot = base->get_config().get_installroot_option().get_value();
    if (char * dir = getenv("TEST_OBS_CONFIG_DIR")) {
        result = installroot.empty() ? dir : installroot / dir;
        return result / "yum.repos.d";
    }
    return installroot.empty() ? OBS_REPO_DIRECTORY : installroot / OBS_REPO_DIRECTORY;
}


static void parse_project_repo_spec(
    libdnf5::Base & base,
    const std::string & spec,
    std::string * hostname = nullptr,
    std::string * project = nullptr,
    std::string * reponame = nullptr) {
    /// Convert from project_repo_spec to hostname, project, and reponame
    /// Example:
    ///  spec == build.opensuse.org/home:user:subproject/reponame
    ///  hubspec = build.opensuse.org
    ///  project = home:user:subproject
    ///  reponame = reponame
    ///
    /// The hubspec can be a full hostname (e.g. build.opensuse.org),
    /// a short name (e.g. opensuse), or empty. See get_hub_hostname()
    /// for details.

    std::smatch match;
    if (!std::regex_match(spec, match, std::regex("^(([^/]+)/)?([^/]+)/([^/]+)$")))
        throw std::runtime_error(_("Can't parse OBS repo spec: ") + spec);
    enum {
        HUBSPEC = 2,
        PROJECT = 3,
        REPONAME = 4,
    };

    if (hostname) {
        auto config = std::make_unique<ObsConfig>(base);
        *hostname = config->get_hub_hostname(match[HUBSPEC]);
    }
    if (project)
        *project = match[PROJECT];
    if (reponame)
        *reponame = match[REPONAME];
}


static void parse_repo_id(
    const std::string repo_id,
    std::string * hostname = nullptr,
    std::string * project = nullptr,
    std::string * reponame = nullptr) {
    /// Convert from obs repo id to hostname, project, and reponame
    /// Example:
    ///  repo_id == obs:build.opensuse.org:home:user:subproject:reponame
    ///  hostname = build.opensuse.org
    ///  project = home:user:subproject
    ///  reponame = reponame

    std::smatch match;
    if (!std::regex_match(repo_id, match, std::regex("^obs:([^/:]+):([^/]+):([^/:]+)$")))
        throw std::runtime_error(_("Can't parse OBS repo id: ") + repo_id);
    enum {
        HOSTNAME = 1,
        PROJECT = 2,
        REPONAME = 3,
    };
    if (hostname)
        *hostname = match[HOSTNAME];
    if (project)
        *project = match[PROJECT];
    if (reponame)
        *reponame = match[REPONAME];
}


static std::string build_repo_id(
    std::string hostname,
    std::string project,
    std::string reponame) {
    /// This is the "repo id".  Example:
    /// obs:build.opensuse.org:home:user:subproject:reponame
    return "obs:" + hostname + ":" + project + ":" + reponame;
}


/// Create an ObsRepo from a project repo spec. This will download the
/// repo file from the specified OBS instance.
ObsRepo::ObsRepo(
    libdnf5::Base & base,
    const std::string & project_repo_spec) {

    this->base = &base;

    parse_project_repo_spec(base, project_repo_spec, &hostname, &project, &reponame);

    std::string repo_filename("_" + get_id() + ".repo");
    repo_file_path = obs_repo_directory(this->base) / repo_filename;

    auto tmp_file = libdnf5::utils::fs::TempFile("/tmp", "dnf5-obs-plugin");

    auto config = std::make_unique<ObsConfig>(base);

    auto repo_file_url = config->get_download_url(hostname, project, reponame) + "/" + project + ".repo";

    try {
        download_file(base, repo_file_url, tmp_file.get_path());
    } catch (const libdnf5::repo::FileDownloadError & e) {
        auto html_url = config->get_html_repository_state_url(hostname, project, reponame);
        auto html = std::make_unique<Html>(base, html_url);
        auto download_url = html->get_download_url();

        if (download_url.empty())
            throw std::runtime_error(_("Failed to get repo for spec: ") + project_repo_spec);

        repo_file_url = download_url + "/" + project + ".repo";

        download_file(base, repo_file_url, tmp_file.get_path());
    }

    parser.read(tmp_file.get_path().native());

    /// OBS repo files have a single section named for the project,
    /// with ":" replaced by "_". We want to use our full id instead,
    /// so we copy the section contents into a new correctly named section
    /// and remove the old section.
    parser.add_section(get_id());
    auto project_id = std::regex_replace(project, std::regex(":"), "_");
    for (const auto & [section_id, opts] : parser.get_data()) {
        if (section_id == get_id())
            continue;
        if (section_id != project_id)
            throw std::runtime_error(libdnf5::utils::sformat(_("Unexpected repository section '{}'"), section_id));
        for (const auto & [key, value] : opts)
            // Skip comments and blank lines
            if (key[0] != '#')
                parser.set_value(get_id(), key, value);
    }
    parser.remove_section(project_id);
}


/// Create an ObsRepo from an existing repo file. This does not
/// communicate with the OBS instance.
ObsRepo::ObsRepo(
    libdnf5::Base & base,
    const std::filesystem::path & repo_file_path) {

    this->base = &base;
    this->repo_file_path = repo_file_path;

    parser.read(repo_file_path.native());

    std::string my_repo_id("");
    for (const auto & [repo_id, opts] : parser.get_data()) {
        if (my_repo_id != "")
            throw std::runtime_error(libdnf5::utils::sformat(_("Repository file '{}' has multiple sections, OBS requires only one"), repo_file_path.native()));
        my_repo_id = repo_id;
    }
    if (my_repo_id == "")
        throw std::runtime_error(libdnf5::utils::sformat(_("Repository file '{}' has no sections"), repo_file_path.native()));

    parse_repo_id(my_repo_id, &hostname, &project, &reponame);
}


bool ObsRepo::is_enabled() const {
    if (parser.has_option(get_id(), "enabled"))
        return parser.get_value(get_id(), "enabled") == "1";
    return false;
}


void ObsRepo::enable() {
    parser.set_value(get_id(), "enabled", "1");
}


void ObsRepo::disable() {
    parser.set_value(get_id(), "enabled", "0");
}


std::string ObsRepo::get_id() const {
    return build_repo_id(hostname, project, reponame);
}


void ObsRepo::remove() const {
    std::string path = get_repo_file_path().native();
    if (-1 == unlink(path.c_str())) {
        std::string msg = libdnf5::utils::sformat(_("Can't remove the {} repo file"), path);
        throw std::runtime_error(msg);
    }
    std::cout << libdnf5::utils::sformat(_("Repo file {} successfully removed"), path) << std::endl;
}


void ObsRepo::save() {
    std::string path = get_repo_file_path().native();
    parser.write(path, false);

    std::filesystem::permissions(
        get_repo_file_path(),
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write | std::filesystem::perms::group_read |
            std::filesystem::perms::others_read,
        std::filesystem::perm_options::add);
}


void ObsRepo::save_interactive() {
    std::cerr << OBS_THIRD_PARTY_WARNING;
    if (libdnf5::cli::utils::userconfirm::userconfirm(base->get_config()))
        save();
}


static bool is_obs_repofile(const std::filesystem::path & repofile) {
    return repofile.filename().native().starts_with("_obs:");
}


static void warn_weird_obs_repo(libdnf5::repo::RepoWeakPtr dnf_repo) {
    /// Throw a warning for every obs-repofile-related repo ID which doesn't
    /// follow the pre-defined format.

    std::string dnf_repo_id = dnf_repo->get_id();
    std::string dnf_repo_file_path = dnf_repo->get_repo_file_path();
    if (!dnf_repo_id.starts_with("obs:")) {
        warning(_("Repo id '{}' in '{}' is suspicious"), dnf_repo_id.c_str(), dnf_repo_file_path.c_str());
        return;
    }

    // Check that the repo filename matches the repo ID
    std::string expected_filename = "_" + dnf_repo_id + ".repo";
    std::filesystem::path dnf_repo_path(dnf_repo_file_path);
    if (expected_filename != dnf_repo_path.filename().string())
        warning(_("Repo id '{}' doesn't belong to '{}' repofile"), dnf_repo_id.c_str(), dnf_repo_file_path.c_str());
}


void installed_obs_repositories(libdnf5::Base & base, ObsRepoCallback cb) {
    std::map<std::string, ObsRepo> obs_repos;

    libdnf5::repo::RepoQuery query(base);
    for (const auto & dnf_repo : query.get_data()) {
        std::filesystem::path dnf_repo_file_path = dnf_repo->get_repo_file_path();

        if (!std::filesystem::exists(dnf_repo_file_path) || !is_obs_repofile(dnf_repo_file_path))
            continue;

        warn_weird_obs_repo(dnf_repo);
        obs_repos.emplace(dnf_repo_file_path.string(), ObsRepo(base, dnf_repo_file_path));
    }

    for (auto & [key, obs_repo] : obs_repos)
        cb(obs_repo);
}


static std::string repo_id_from_project_repo_spec(libdnf5::Base & base, const std::string & project_repo_spec_string) {
    std::string hostname, project, reponame;
    parse_project_repo_spec(base, project_repo_spec_string, &hostname, &project, &reponame);
    return build_repo_id(hostname, project, reponame);
}


class RepoDisableCB : public ObsRepoCallback {
private:
    std::string id;

public:
    int count = 0;
    explicit RepoDisableCB(const std::string & id) : id(id) {}
    dnf5::ObsRepoCallback disable = [&](dnf5::ObsRepo & repo) {
        if (id == repo.get_id()) {
            repo.disable();
            repo.save();
            count++;
            std::cout << libdnf5::utils::sformat(_("OBS repository '{}' in '{}' disabled."), repo.get_id(), repo.get_repo_file_path().native()) << std::endl;
        }
    };
};


void obs_repo_disable(libdnf5::Base & base, const std::string & project_repo_spec_string) {
    auto repo_id = repo_id_from_project_repo_spec(base, project_repo_spec_string);
    auto disabler = RepoDisableCB(repo_id);
    installed_obs_repositories(base, disabler.disable);

    if (disabler.count == 0)
        throw std::runtime_error(libdnf5::utils::sformat(_("Repository '{}' not found on this system"), repo_id));
}


class RepoRemoveCB : public ObsRepoCallback {
private:
    std::string id;

public:
    int count = 0;
    explicit RepoRemoveCB(const std::string & id) : id(id) {}
    dnf5::ObsRepoCallback remove = [&](dnf5::ObsRepo & repo) {
        if (id == repo.get_id()) {
            repo.remove();
            count++;
            std::cout << libdnf5::utils::sformat(_("OBS repository '{}' in '{}' removed."), repo.get_id(), repo.get_repo_file_path().native()) << std::endl;
        }
    };
};

void obs_repo_remove(libdnf5::Base & base, const std::string & project_repo_spec_string) {
    auto repo_id = repo_id_from_project_repo_spec(base, project_repo_spec_string);
    auto remover = RepoRemoveCB(repo_id);
    installed_obs_repositories(base, remover.remove);

    if (remover.count == 0)
        throw std::runtime_error(libdnf5::utils::sformat(_("Repository '{}' not found on this system"), repo_id));
}


}  // namespace dnf5
