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


#include "support.hpp"

#include "libdnf/rpm/package_query.hpp"

#include "utils.hpp"

#include <filesystem>
#include <map>


// Static map (class_name -> cache_dir) that allows re-using cache dirs among test cases in a class.
// Prevents creating solv files over and over again.
static std::map<std::string, std::unique_ptr<libdnf::utils::TempDir>> cache_dirs;


void LibdnfTestCase::add_repo(const std::string & repoid, const std::string & repo_path) {
    auto repo = repo_sack->new_repo(repoid);

    // set the repo baseurl
    repo->get_config().baseurl().set(libdnf::Option::Priority::RUNTIME, "file://" + repo_path);

    // load repository into rpm::Repo
    repo->load();

    // load repo content into rpm::PackageSack
    sack->load_repo(*repo.get(),
        libdnf::rpm::PackageSack::LoadRepoFlags::USE_FILELISTS |
        libdnf::rpm::PackageSack::LoadRepoFlags::USE_OTHER |
        libdnf::rpm::PackageSack::LoadRepoFlags::USE_PRESTO |
        libdnf::rpm::PackageSack::LoadRepoFlags::USE_UPDATEINFO
    );
}


void LibdnfTestCase::add_repo_repomd(const std::string & repoid) {
    std::filesystem::path repo_path = PROJECT_SOURCE_DIR "/test/data/repos-repomd";
    repo_path /= repoid;
    add_repo(repoid, repo_path);
}


void LibdnfTestCase::add_repo_rpm(const std::string & repoid) {
    std::filesystem::path repo_path = PROJECT_BINARY_DIR "/test/data/repos-rpm";
    repo_path /= repoid;
    add_repo(repoid, repo_path);
}


void LibdnfTestCase::add_repo_solv(const std::string & repoid) {
    std::filesystem::path repo_path = PROJECT_SOURCE_DIR "/test/data/repos-solv";
    repo_path /= repoid + ".repo";
    repo_sack->new_repo_from_libsolv_testcase(repoid.c_str(), repo_path.native());
}


libdnf::rpm::Package LibdnfTestCase::get_pkg(const std::string & nevra, bool installed) {
    libdnf::rpm::PackageQuery query(*base);
    query.filter_nevra({nevra});
    if (installed) {
        query.filter_installed();
    } else {
        query.filter_available();
    }
    return first_query_pkg(query, nevra + " (" + (installed ? "installed" : "available") + ")");
}


libdnf::rpm::Package LibdnfTestCase::get_pkg(const std::string & nevra, const char * repo) {
    libdnf::rpm::PackageQuery query(*base);
    query.filter_nevra({nevra});
    query.filter_repoid({repo});
    return first_query_pkg(query, nevra + " (repo: " + repo + ")");
}


libdnf::rpm::Package LibdnfTestCase::first_query_pkg(libdnf::rpm::PackageQuery & query, const std::string & what) {
    if (query.empty()) {
        CPPUNIT_FAIL("No package \"" + what + "\" found. All sack packages:" + \
            list_pkg_infos(libdnf::rpm::PackageQuery(*base)));
    } else if (query.size() > 1) {
        CPPUNIT_FAIL("More than one package matching \"" + what + "\" found:" + list_pkg_infos(query));
    }

    return *query.begin();
}


void LibdnfTestCase::setUp() {
    TestCaseFixture::setUp();

    temp = std::make_unique<libdnf::utils::TempDir>(
        "libdnf_unittest_",
        std::vector<std::string>{"installroot"}
    );
    base = std::make_unique<libdnf::Base>();

    // set installroot to a temp directory
    base->get_config().installroot().set(libdnf::Option::Priority::RUNTIME, temp->get_path() / "installroot");

    // use the shared cache dir (see cache_dirs comment for more details)
    auto class_name = typeid(*this).name();
    auto it = cache_dirs.find(class_name);
    if (it == cache_dirs.end()) {
        cache_dirs.insert({class_name, std::make_unique<libdnf::utils::TempDir>("libdnf_unittest_")});
    }
    base->get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, cache_dirs.at(class_name)->get_path());

    repo_sack = base->get_repo_sack();
    sack = base->get_rpm_package_sack();
}

void LibdnfTestCase::dump_debugdata() {
    sack->dump_debugdata("debugdata");
}
