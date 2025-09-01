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

#include "CoprRepoTest.hpp"

#include "copr_config.hpp"
#include "copr_repo.hpp"

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
/// dnf5-plugins/copr_plugin/download_file.cpp
void download_file(libdnf5::Base &, const std::string & url, const std::filesystem::path & path) {
    std::filesystem::path input_filename = TEST_DATADIR;
    ofstream output;
    ifstream input;
    output.open(path);
    if (url.find("/api_3/rpmrepo/praiskup/ping/") != std::string::npos)
        input_filename /= "praiskup-ping.json";
    else if (url.find("/api_3/rpmrepo/phracek/PyCharm/") != std::string::npos)
        input_filename /= "phracek-pycharm-f37.json";
    else
        throw runtime_error("sorry, unknown url " + url);
    input.open(input_filename);
    output << input.rdbuf();
}

std::string repo_to_string(const dnf5::CoprRepo & repo) {
    std::stringstream repo_stream;
    repo_stream << repo;
    return repo_stream.str();
}

void CoprRepoTest::prepare_env() {
    installroot = temp_dir->get_path() / "installroot";
    std::filesystem::create_directory(installroot);
    confdir = installroot / "etc";
    std::filesystem::create_directory(confdir);
    repodir = confdir / "yum.repos.d";
    std::filesystem::create_directory(repodir);
    std::filesystem::path test_data = TEST_DATADIR;
    test_data /= "etc-fixes/f38-normal";
    std::filesystem::copy(
        test_data,
        confdir,
        std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
    setenv("TEST_COPR_CONFIG_DIR", confdir.c_str(), 1);
}


void CoprRepoTest::install_repofile(const std::string & name, const std::string & dest) {
    std::string repo_contents;
    repo_contents = get_data_file_contents(name);
    std::filesystem::path repofile = repodir / dest;
    std::ofstream repostream(repofile);
    repostream << repo_contents;
    repostream.close();
}


void CoprRepoTest::test_list_sth() {
    prepare_env();

    install_repofile("praiskup-ping-fedora-38-x86_64.repo", "_copr:copr.fedorainfracloud.org:praiskup:ping.repo");
    install_repofile("jdoe-beakerlib-fedora-38-x86_64.repo", "_copr:copr.example.com:jdoe:beakerlib.repo");
    install_repofile("group_mock-mock-fedora-38-x86_64.repo", "_copr:copr.fedorainfracloud.org:group_mock:mock.repo");

    reload_repofiles();

    class RepoLister {
    public:
        std::set<std::string> found;
        dnf5::CoprRepoCallback lister = [&](dnf5::CoprRepo & cr) {
            if (cr.get_id() == "copr.fedorainfracloud.org/praiskup/ping") {
                CPPUNIT_ASSERT(cr.is_multilib());
                CPPUNIT_ASSERT(cr.has_external_deps());
                CPPUNIT_ASSERT(cr.is_enabled());
            } else if (cr.get_id() == "copr.example.com/jdoe/beakerlib") {
                CPPUNIT_ASSERT(!cr.is_multilib());
                CPPUNIT_ASSERT(!cr.has_external_deps());
                CPPUNIT_ASSERT(cr.is_enabled());
            } else if (cr.get_id() == "copr.example.com/@mock/mock") {
                CPPUNIT_ASSERT(!cr.is_multilib());
                CPPUNIT_ASSERT(!cr.has_external_deps());
                CPPUNIT_ASSERT(!cr.is_enabled());
            }
            found.insert(cr.get_id());
        };
    };

    RepoLister list;
    dnf5::installed_copr_repositories(base, list.lister);

    std::set<std::string> expected;
    expected.insert("copr.fedorainfracloud.org/praiskup/ping");
    expected.insert("copr.example.com/jdoe/beakerlib");
    expected.insert("copr.fedorainfracloud.org/@mock/mock");
    CPPUNIT_ASSERT(expected == list.found);
}


void CoprRepoTest::test_enable() {
    prepare_env();
    std::unique_ptr<dnf5::CoprConfig> config = std::make_unique<dnf5::CoprConfig>(base);
    dnf5::CoprRepo repo(base, config, "fedora/praiskup/ping", "");
    auto expected = get_data_file_contents("praiskup-ping-fedora-38-x86_64.repo");
    CPPUNIT_ASSERT_EQUAL(expected, repo_to_string(repo));
    repo.save();
    auto saved_file = confdir / "yum.repos.d" / "_copr:copr.fedorainfracloud.org:praiskup:ping.repo";
    CPPUNIT_ASSERT_EQUAL(expected, load_file_as_string(saved_file));
}


void CoprRepoTest::reload_repofiles() {
    /// reload repofiles so further queries give us correct results
    base.get_repo_sack()->create_repos_from_system_configuration();
}

void CoprRepoTest::test_disable() {
    prepare_env();
    std::string disabled_datafile = "praiskup-ping-fedora-38-x86_64.repo";
    std::string disabled_repofile = "_copr:copr.fedorainfracloud.org:praiskup:ping.repo";
    install_repofile(disabled_datafile, disabled_repofile);
    install_repofile("jdoe-beakerlib-fedora-38-x86_64.repo", "_copr:copr.example.com:jdoe:beakerlib.repo");
    reload_repofiles();
    dnf5::copr_repo_disable(base, "praiskup/ping");
    auto saved_file = confdir / "yum.repos.d" / disabled_repofile;
    auto expected = get_data_file_contents(disabled_datafile);
    expected = std::regex_replace(expected, std::regex("enabled=1"), "enabled=0");
    CPPUNIT_ASSERT_EQUAL(expected, load_file_as_string(saved_file));
}


void CoprRepoTest::test_remove() {
    prepare_env();
    install_repofile("praiskup-ping-fedora-38-x86_64.repo", "_copr:copr.fedorainfracloud.org:praiskup:ping.repo");
    install_repofile("jdoe-beakerlib-fedora-38-x86_64.repo", "_copr:copr.example.com:jdoe:beakerlib.repo");
    install_repofile("group_mock-mock-fedora-38-x86_64.repo", "_copr:copr.fedorainfracloud.org:group_mock:mock.repo");
    reload_repofiles();

    dnf5::copr_repo_remove(base, "@mock/mock");
    dnf5::copr_repo_remove(base, "copr.fedorainfracloud.org/praiskup/ping");

    CPPUNIT_ASSERT(!std::filesystem::exists(repodir / "_copr:copr.fedorainfracloud.org:praiskup:ping.repo"));
    CPPUNIT_ASSERT(std::filesystem::exists(repodir / "_copr:copr.example.com:jdoe:beakerlib.repo"));
    CPPUNIT_ASSERT(!std::filesystem::exists(repodir / "_copr:copr.fedorainfracloud.org:group_mock:mock.repo"));
}


void CoprRepoTest::test_remove_old_file() {
    prepare_env();
    std::string removed_repofile = "_copr_phracek-PyCharm.repo";
    install_repofile("_copr_phracek-PyCharm.repo", removed_repofile);
    reload_repofiles();

    std::unique_ptr<dnf5::CoprConfig> config = std::make_unique<dnf5::CoprConfig>(base);

    // use an explicit chroot as a bonus test
    dnf5::CoprRepo repo(base, config, "copr.fedorainfracloud.org/phracek/PyCharm", "fedora-37-x86_64");

    auto expected = get_data_file_contents("phracek-PyCharm-fedora-37-x86_64.repo");
    CPPUNIT_ASSERT_EQUAL(expected, repo_to_string(repo));
    repo.save();
    auto saved_file = confdir / "yum.repos.d" / "_copr:copr.fedorainfracloud.org:phracek:PyCharm.repo";
    CPPUNIT_ASSERT_EQUAL(expected, load_file_as_string(saved_file));

    CPPUNIT_ASSERT(std::filesystem::exists(repodir / "_copr:copr.fedorainfracloud.org:phracek:PyCharm.repo"));
    CPPUNIT_ASSERT(!std::filesystem::exists(repodir / removed_repofile));
}


CPPUNIT_TEST_SUITE_REGISTRATION(CoprRepoTest);
