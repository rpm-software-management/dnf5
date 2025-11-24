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


/// this is a mocked replacement for
/// dnf5-plugins/obs_plugin/download_file.cpp
void download_file(libdnf5::Base &, const std::string & url, const std::filesystem::path & path) {
    std::filesystem::path input_filename = TEST_DATADIR;
    ofstream output;
    ifstream input;
    output.open(path);
    if (url.find("/project/repository_state/home:ddstreet42:systemd/Fedora_Rawhide") != std::string::npos)
        input_filename /= "Repository State of home_ddstreet42_systemd - openSUSE Build Service.html";
    else if (url.find("/repositories/home:/ddstreet42:/systemd/Fedora_Rawhide/home:ddstreet42:systemd.repo") != std::string::npos)
        input_filename /= "home_ddstreet42_systemd.repo";
    else
        throw runtime_error("sorry, unknown url " + url);
    input.open(input_filename);
    output << input.rdbuf();
}

void ObsRepoTest::prepare_env() {
    installroot = temp_dir->get_path() / "installroot";
    std::filesystem::create_directory(installroot);
    confdir = installroot / "etc";
    std::filesystem::create_directory(confdir);
    repodir = confdir / "yum.repos.d";
    std::filesystem::create_directory(repodir);
    std::filesystem::path test_data = TEST_DATADIR;
    test_data /= "etc-fixes/f43-normal";
    std::filesystem::copy(
        test_data,
        confdir,
        std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
    setenv("TEST_OBS_CONFIG_DIR", confdir.c_str(), 1);
}


void ObsRepoTest::install_repofile(const std::string & name, const std::string & dest) {
    std::string repo_contents;
    repo_contents = get_data_file_contents(name);
    std::filesystem::path repofile = repodir / dest;
    std::ofstream repostream(repofile);
    repostream << repo_contents;
    repostream.close();
}


void ObsRepoTest::test_list_sth() {
    prepare_env();

    install_repofile("openSUSE_Tools.repo", "_obs:build.opensuse.org:openSUSE:Tools:Fedora_43.repo");
    install_repofile("home_user.repo", "_obs:build.opensuse.org:home:user:Fedora_40.repo");
    install_repofile("home_ddstreet42.repo", "_obs:lsgobs.ddstreet.org:home:ddstreet42:AzureLinux_3.repo");

    reload_repofiles();

    class RepoLister {
    public:
        std::set<std::string> found;
        dnf5::ObsRepoCallback lister = [&](dnf5::ObsRepo & cr) {
            if (cr.get_id() == "obs:build.opensuse.org:openSUSE:Tools:Fedora_43") {
                CPPUNIT_ASSERT(cr.is_enabled());
            } else if (cr.get_id() == "obs:build.opensuse.org:home:ddstreet42:systemd:Fedora_Rawhide") {
                CPPUNIT_ASSERT(cr.is_enabled());
            } else if (cr.get_id() == "obs:lsgobs.ddstreet.org:home:ddstreet42:AzureLinux_3") {
                CPPUNIT_ASSERT(!cr.is_enabled());
            }
            found.insert(cr.get_id());
        };
    };

    RepoLister list;
    dnf5::installed_obs_repositories(base, list.lister);

    std::set<std::string> expected;
    expected.insert("obs:build.opensuse.org:openSUSE:Tools:Fedora_43");
    expected.insert("obs:build.opensuse.org:home:user:Fedora_40");
    expected.insert("obs:lsgobs.ddstreet.org:home:ddstreet42:AzureLinux_3");
    CPPUNIT_ASSERT(expected == list.found);
}


void ObsRepoTest::test_enable() {
    prepare_env();
    auto repo = dnf5::ObsRepo(base, (std::string)"opensuse/home:ddstreet42:systemd/Fedora_Rawhide");
    repo.enable();
    repo.save();
    auto saved_file = confdir / "yum.repos.d" / "_obs:build.opensuse.org:home:ddstreet42:systemd:Fedora_Rawhide.repo";
    auto expected = get_data_file_contents("home_ddstreet42_systemd.repo");
    CPPUNIT_ASSERT_EQUAL(expected, load_file_as_string(saved_file));
}


void ObsRepoTest::reload_repofiles() {
    /// reload repofiles so further queries give us correct results
    base.get_repo_sack()->create_repos_from_system_configuration();
}

void ObsRepoTest::test_disable() {
    prepare_env();
    std::string disabled_datafile = "home_user.repo";
    std::string disabled_repofile = "_obs:build.opensuse.org:home:user:Fedora_40.repo";
    install_repofile(disabled_datafile, disabled_repofile);
    install_repofile("home_ddstreet42.repo", "_obs:lsgobs.ddstreet.org:home:ddstreet42:AzureLinux_3.repo");
    reload_repofiles();
    dnf5::obs_repo_disable(base, "home:user/Fedora_40");
    auto saved_file = confdir / "yum.repos.d" / disabled_repofile;
    auto expected = get_data_file_contents(disabled_datafile);
    expected = std::regex_replace(expected, std::regex("enabled=1"), "enabled=0");
    CPPUNIT_ASSERT_EQUAL(expected, load_file_as_string(saved_file));
}


void ObsRepoTest::test_remove() {
    prepare_env();
    install_repofile("home_ddstreet42.repo", "_obs:lsgobs.ddstreet.org:home:ddstreet42:AzureLinux_3.repo");
    install_repofile("system_systemd.repo", "_obs:build.opensuse.org:system:systemd:openSUSE_Tumbleweed.repo");
    install_repofile("home_user.repo", "_obs:build.opensuse.org:home:user:Fedora_40.repo");
    reload_repofiles();

    dnf5::obs_repo_remove(base, "system:systemd/openSUSE_Tumbleweed");
    dnf5::obs_repo_remove(base, "lsgobs.ddstreet.org/home:ddstreet42/AzureLinux_3");

    CPPUNIT_ASSERT(!std::filesystem::exists(repodir / "_obs:lsgobs.ddstreet.org:home:ddstreet42:AzureLinux_3.repo"));
    CPPUNIT_ASSERT(!std::filesystem::exists(repodir / "_obs:build.opensuse.org:system:systemd:openSUSE_Tumbleweed.repo"));
    CPPUNIT_ASSERT(std::filesystem::exists(repodir / "_obs:build.opensuse.org:home:user:Fedora_40.repo"));
}


CPPUNIT_TEST_SUITE_REGISTRATION(ObsRepoTest);
