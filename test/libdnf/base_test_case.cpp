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


#include "base_test_case.hpp"

#include "utils.hpp"
#include "utils/string.hpp"

#include "libdnf/comps/group/group.hpp"
#include "libdnf/comps/group/query.hpp"
#include "libdnf/rpm/nevra.hpp"
#include "libdnf/rpm/package_query.hpp"

#include <filesystem>
#include <map>


using libdnf::utils::sformat;


libdnf::repo::RepoWeakPtr BaseTestCase::add_repo(const std::string & repoid, const std::string & repo_path, bool load) {
    auto repo = repo_sack->create_repo(repoid);

    repo->get_config().baseurl().set(libdnf::Option::Priority::RUNTIME, "file://" + repo_path);

    if (load) {
        repo->fetch_metadata();
        repo->load();
    }

    return repo;
}


libdnf::repo::RepoWeakPtr BaseTestCase::add_repo_repomd(const std::string & repoid, bool load) {
    std::filesystem::path repo_path = PROJECT_SOURCE_DIR "/test/data/repos-repomd";
    repo_path /= repoid;
    return add_repo(repoid, repo_path, load);
}


libdnf::repo::RepoWeakPtr BaseTestCase::add_repo_rpm(const std::string & repoid, bool load) {
    std::filesystem::path repo_path = PROJECT_BINARY_DIR "/test/data/repos-rpm";
    repo_path /= repoid;
    return add_repo(repoid, repo_path, load);
}


libdnf::repo::RepoWeakPtr BaseTestCase::add_repo_solv(const std::string & repoid) {
    std::filesystem::path repo_path = PROJECT_SOURCE_DIR "/test/data/repos-solv";
    repo_path /= repoid + ".repo";
    return repo_sack->create_repo_from_libsolv_testcase(repoid.c_str(), repo_path.native());
}


libdnf::advisory::Advisory BaseTestCase::get_advisory(const std::string & name) {
    // This is used for testing queries as well, hence we don't use the AdvisoryQuery facility for filtering
    libdnf::advisory::AdvisorySet advisories = libdnf::advisory::AdvisoryQuery(base);

    libdnf::advisory::AdvisorySet found(base);
    for (auto advisory : advisories) {
        if (advisory.get_name() == name) {
            found.add(advisory);
        }
    }

    if (found.empty()) {
        CPPUNIT_FAIL(sformat("No advisory \"{}\" found. All pool advisories:{}", name, to_string(advisories)));
    } else if (found.size() > 1) {
        CPPUNIT_FAIL(sformat("More than one advisory matching \"{}\" found:{}", name, to_string(advisories)));
    }

    return *found.begin();
}


libdnf::comps::Group BaseTestCase::get_group(const std::string & groupid, bool installed) {
    // This is used for testing queries as well, hence we don't use the GroupQuery facility for filtering
    libdnf::Set<libdnf::comps::Group> groups = libdnf::comps::GroupQuery(base);

    std::set<libdnf::comps::Group> found;
    for (auto group : groups) {
        if (group.get_groupid() == groupid && group.get_installed() == installed) {
            found.insert(group);
        }
    }

    if (found.empty()) {
        CPPUNIT_FAIL(sformat("No group \"{}\" found. All pool groups:{}", groupid, to_string(groups)));
    } else if (found.size() > 1) {
        CPPUNIT_FAIL(sformat("More than one group matching \"{}\" found:{}", groupid, to_string(groups)));
    }

    return *found.begin();
}


libdnf::rpm::Package BaseTestCase::get_pkg(const std::string & nevra, bool installed) {
    libdnf::rpm::PackageQuery query(base);
    query.filter_nevra({nevra});
    if (installed) {
        query.filter_installed();
    } else {
        query.filter_available();
    }
    return first_query_pkg(query, nevra + " (" + (installed ? "installed" : "available") + ")");
}


libdnf::rpm::Package BaseTestCase::get_pkg(const std::string & nevra, const char * repo) {
    libdnf::rpm::PackageQuery query(base);
    query.filter_nevra({nevra});
    query.filter_repo_id({repo});
    return first_query_pkg(query, nevra + " (repo: " + repo + ")");
}


libdnf::rpm::Package BaseTestCase::get_pkg_i(const std::string & nevra, size_t index) {
    libdnf::rpm::PackageQuery query(base);
    query.filter_nevra({nevra});

    if (query.size() <= index) {
        CPPUNIT_FAIL(
            sformat("Package index {} out of bounds for \"{}\", query packages:{}", index, nevra, to_string(query)));
    }

    auto it = query.begin();
    while (index-- > 0) {
        ++it;
    }

    return *it;
}


libdnf::rpm::Package BaseTestCase::add_system_pkg(
    const std::string & relative_path, libdnf::transaction::TransactionItemReason reason) {
    if (reason != libdnf::transaction::TransactionItemReason::UNKNOWN) {
        // parse out the NA from the package path to set the reason for the installed package
        auto filename_toks = libdnf::utils::string::split(relative_path, "/");
        auto basename_toks = libdnf::utils::string::rsplit(filename_toks.back(), ".", 2);
        auto nevras = libdnf::rpm::Nevra::parse(basename_toks.front());
        CPPUNIT_ASSERT_MESSAGE("Couldn't parse NEVRA from package path: \"" + relative_path + "\"", !nevras.empty());
        auto na = nevras[0].get_name() + "." + nevras[0].get_arch();

        base.get_system_state().set_reason(na, reason);
    }

    return sack->add_system_package(PROJECT_BINARY_DIR "/test/data/" + relative_path, false);
}


libdnf::rpm::Package BaseTestCase::add_cmdline_pkg(const std::string & relative_path) {
    return sack->add_cmdline_package(PROJECT_BINARY_DIR "/test/data/" + relative_path, false);
}


libdnf::rpm::Package BaseTestCase::first_query_pkg(libdnf::rpm::PackageQuery & query, const std::string & what) {
    if (query.empty()) {
        CPPUNIT_FAIL(
            sformat("No package \"{}\" found. All sack packages:{}", what, to_string(libdnf::rpm::PackageQuery(base))));
    } else if (query.size() > 1) {
        CPPUNIT_FAIL(sformat("More than one package matching \"{}\" found:{}", what, to_string(query)));
    }

    return *query.begin();
}


void BaseTestCase::setUp() {
    TestCaseFixture::setUp();

    temp = std::make_unique<libdnf::utils::fs::TempDir>("libdnf_unittest");
    std::filesystem::create_directory(temp->get_path() / "installroot");

    base.get_config().installroot().set(libdnf::Option::Priority::RUNTIME, temp->get_path() / "installroot");
    base.get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, temp->get_path() / "cache");

    base.setup();

    repo_sack = base.get_repo_sack();
    sack = base.get_rpm_package_sack();
}

void BaseTestCase::dump_debugdata() {
    repo_sack->dump_debugdata("debugdata");
}
