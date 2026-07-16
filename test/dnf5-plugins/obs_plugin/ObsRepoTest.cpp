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

#include "ObsRepoTest.hpp"

#include "obs_config.hpp"
#include "obs_repo.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/common/exception.hpp>
#include <libdnf5/repo/file_downloader_errors.hpp>

#include <filesystem>
#include <regex>
#include <sstream>


using namespace std;


std::string load_file_as_string(const std::filesystem::path & filename) {
    ifstream input;
    input.open(filename);
    std::stringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string get_data_file_contents(const std::string & filename) {
    filesystem::path path = TEST_DATADIR;
    path /= filename;
    return load_file_as_string(path);
}


std::string get_repo_filename_for_url(std::string url) {
    const std::string OPENSUSE_PREFIX = "https://download.opensuse.org";
    const std::string STANDARD_PREFIX = "http://standard.ddstreet.org:82";
    const std::string NONSTANDARD_PREFIX = "https://nonstandard.ddstreet.org";

    if (url.find(OPENSUSE_PREFIX) == 0) {
        url = url.erase(0, OPENSUSE_PREFIX.length());
        if (url.find(":443") == 0)
            url = url.erase(0, std::string(":443").length());
        if (url.find("/repositories/home:/ddstreet42:/systemd/Fedora_Rawhide/") == 0 ||
            url.find("/repositories/home:/user/Fedora_42/") == 0 ||
            url.find("/repositories/openSUSE:/Tools/15.7/") == 0 ||
            url.find("/repositories/system:/systemd/Fedora_Rawhide/") == 0)
            return url.substr(url.rfind('/') + 1);
    } else if (url.find(STANDARD_PREFIX) == 0) {
        url = url.erase(0, STANDARD_PREFIX.length());
        if (url.find("/home:/ddstreet42/Fedora_43/") == 0)
            return url.substr(url.rfind('/') + 1);
    } else if (url.find(NONSTANDARD_PREFIX) == 0) {
        url = url.erase(0, NONSTANDARD_PREFIX.length());
        if (url.find(":443") == 0)
            url = url.erase(0, std::string(":443").length());
        if (url.find("/download/home:/ddstreet44/Fedora_39/") == 0)
            return url.substr(url.rfind('/') + 1);
    }
    return std::string("");
}


/// this is a mocked replacement for
/// dnf5-plugins/obs_plugin/download_file.cpp
void download_file(libdnf5::Base &, const std::string & url, const std::filesystem::path & path) {
    std::filesystem::path input_filename = TEST_DATADIR;
    ofstream output;
    ifstream input;
    output.open(path);
    std::string repo_filename = get_repo_filename_for_url(url);
    if (!repo_filename.empty())
        input_filename /= repo_filename;
    else if (url.find("/project/repository_state/home:ddstreet44/Fedora_39") != std::string::npos)
        input_filename /= "home_ddstreet44.html";
    else if (url.find("http://nonstandard.ddstreet.org:82") != std::string::npos)
        throw libdnf5::repo::FileDownloadError(M_("Try HTML fallback"));
    else
        throw runtime_error("sorry, unknown url " + url);
    input.open(input_filename);
    output << input.rdbuf();
}

void ObsRepoTest::prepare_env() {
    installroot = temp_dir->get_path() / "installroot";
    std::filesystem::path etcdir = installroot / "etc";
    confdir = etcdir / "dnf" / "plugins" / "obs.d";
    repodir = etcdir / "yum.repos.d";
    std::filesystem::create_directories(confdir);
    std::filesystem::create_directories(repodir);
    std::filesystem::path test_data = TEST_DATADIR;
    test_data /= "etc-fixes/f43-normal";
    std::filesystem::copy(
        test_data,
        etcdir,
        std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
    setenv("TEST_OBS_CONFIG_DIR", etcdir.c_str(), 1);
}


void ObsRepoTest::install_configfile(const std::string name) {
    std::filesystem::path path = TEST_DATADIR;
    std::filesystem::copy(path / name, confdir);
}


void ObsRepoTest::install_repofile(const std::string & name) {
    std::filesystem::path path = TEST_DATADIR;
    std::filesystem::copy(path / name, repodir);
}


void ObsRepoTest::test_list_sth() {
    prepare_env();

    install_repofile("_obs:build.opensuse.org:openSUSE:Tools:15.7.repo");
    install_repofile("_obs:build.opensuse.org:home:ddstreet42:systemd:Fedora_Rawhide.repo");
    install_repofile("_obs:standard.ddstreet.org:home:ddstreet42:Fedora_43.repo");

    reload_repofiles();

    class RepoLister {
    public:
        std::set<std::string> found;
        dnf5::ObsRepoCallback lister = [&](dnf5::ObsRepo & cr) {
            if (cr.get_id() == "obs:build.opensuse.org:openSUSE:Tools:15.7") {
                CPPUNIT_ASSERT(cr.is_enabled());
            } else if (cr.get_id() == "obs:build.opensuse.org:home:ddstreet42:systemd:Fedora_Rawhide") {
                CPPUNIT_ASSERT(!cr.is_enabled());
            } else if (cr.get_id() == "obs:standard.ddstreet.org:home:ddstreet42:Fedora_43") {
                CPPUNIT_ASSERT(cr.is_enabled());
            }
            found.insert(cr.get_id());
        };
    };

    RepoLister list;
    dnf5::installed_obs_repositories(base, list.lister);

    std::set<std::string> expected;
    expected.insert("obs:build.opensuse.org:openSUSE:Tools:15.7");
    expected.insert("obs:build.opensuse.org:home:ddstreet42:systemd:Fedora_Rawhide");
    expected.insert("obs:standard.ddstreet.org:home:ddstreet42:Fedora_43");
    CPPUNIT_ASSERT(expected == list.found);
}


void ObsRepoTest::run_test_enable(
    const std::string & project_repo_spec,
    const std::string & repo_filename,
    const std::string & config_filename) {

    prepare_env();
    if (!config_filename.empty())
        install_configfile(config_filename);
    auto repo = dnf5::ObsRepo(base, project_repo_spec);
    repo.enable();
    repo.save();
    auto saved_file = repodir / repo_filename;
    auto expected = get_data_file_contents(repo_filename);
    CPPUNIT_ASSERT_EQUAL(expected, load_file_as_string(saved_file));
}


void ObsRepoTest::test_enable_no_hubspec() {
    run_test_enable("home:user/Fedora_42", "_obs:build.opensuse.org:home:user:Fedora_42.repo");
}


void ObsRepoTest::test_enable_short_hubspec() {
    run_test_enable("opensuse/home:user/Fedora_42", "_obs:build.opensuse.org:home:user:Fedora_42.repo");
}


void ObsRepoTest::test_enable_full_hubspec() {
    run_test_enable("build.opensuse.org/home:user/Fedora_42", "_obs:build.opensuse.org:home:user:Fedora_42.repo");
}


void ObsRepoTest::test_enable_unofficial_short_hubspec() {
    run_test_enable("standard/home:ddstreet42/Fedora_43", "_obs:standard.ddstreet.org:home:ddstreet42:Fedora_43.repo", "standard.conf");
}


void ObsRepoTest::test_enable_unofficial_full_hubspec() {
    run_test_enable("standard.ddstreet.org/home:ddstreet42/Fedora_43", "_obs:standard.ddstreet.org:home:ddstreet42:Fedora_43.repo");
}


void ObsRepoTest::test_enable_html_fallback() {
    run_test_enable("nonstandard.ddstreet.org/home:ddstreet44/Fedora_39", "_obs:nonstandard.ddstreet.org:home:ddstreet44:Fedora_39.repo");
}


void ObsRepoTest::reload_repofiles() {
    /// reload repofiles so further queries give us correct results
    base.get_repo_sack()->create_repos_from_system_configuration();
}

void ObsRepoTest::test_disable() {
    prepare_env();
    std::string repofile = "_obs:build.opensuse.org:home:user:Fedora_42.repo";
    install_repofile(repofile);
    install_repofile("_obs:standard.ddstreet.org:home:ddstreet42:Fedora_43.repo");
    reload_repofiles();
    dnf5::obs_repo_disable(base, "home:user/Fedora_42");
    auto saved_file = repodir / repofile;
    auto expected = get_data_file_contents(repofile);
    expected = std::regex_replace(expected, std::regex("enabled=1"), "enabled=0");
    CPPUNIT_ASSERT_EQUAL(expected, load_file_as_string(saved_file));
}


void ObsRepoTest::test_remove() {
    prepare_env();
    install_repofile("_obs:standard.ddstreet.org:home:ddstreet42:Fedora_43.repo");
    install_repofile("_obs:build.opensuse.org:system:systemd:Fedora_Rawhide.repo");
    install_repofile("_obs:build.opensuse.org:home:user:Fedora_42.repo");
    reload_repofiles();

    dnf5::obs_repo_remove(base, "system:systemd/Fedora_Rawhide");
    dnf5::obs_repo_remove(base, "standard.ddstreet.org/home:ddstreet42/Fedora_43");

    CPPUNIT_ASSERT(!std::filesystem::exists(repodir / "_obs:standard.ddstreet.org:home:ddstreet42:Fedora_43.repo"));
    CPPUNIT_ASSERT(!std::filesystem::exists(repodir / "_obs:build.opensuse.org:system:systemd:Fedora_Rawhide.repo"));
    CPPUNIT_ASSERT(std::filesystem::exists(repodir / "_obs:build.opensuse.org:home:user:Fedora_42.repo"));
}


CPPUNIT_TEST_SUITE_REGISTRATION(ObsRepoTest);
