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


#include "base_test_case.hpp"

#include "logger_redirector.hpp"
#include "test_logger.hpp"
#include "utils.hpp"

#include "libdnf5/utils/fs/temp.hpp"

#include <libdnf5/comps/environment/query.hpp>
#include <libdnf5/comps/group/query.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/rpm/package_query.hpp>

#include <filesystem>
#include <set>


libdnf5::repo::RepoWeakPtr BaseTestCase::add_repo(
    const std::string & repoid, const std::string & repo_path, bool load) {
    auto repo = repo_sack->create_repo(repoid);
    repo->get_config().get_baseurl_option().set("file://" + repo_path);

    if (load) {
        repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);
    }

    return repo;
}


libdnf5::repo::RepoWeakPtr BaseTestCase::add_repo_repomd(const std::string & repoid, bool load) {
    std::filesystem::path repo_path = PROJECT_SOURCE_DIR "/test/data/repos-repomd";
    repo_path /= repoid;
    return add_repo(repoid, repo_path, load);
}


libdnf5::repo::RepoWeakPtr BaseTestCase::add_repo_rpm(const std::string & repoid, bool load) {
    std::filesystem::path repo_path = PROJECT_BINARY_DIR "/test/data/repos-rpm";
    repo_path /= repoid;
    return add_repo(repoid, repo_path, load);
}


libdnf5::repo::RepoWeakPtr BaseTestCase::add_repo_solv(const std::string & repoid) {
    std::filesystem::path repo_path = PROJECT_SOURCE_DIR "/test/data/repos-solv";
    repo_path /= repoid + ".repo";
    return repo_sack->create_repo_from_libsolv_testcase(repoid.c_str(), repo_path.native());
}


libdnf5::advisory::Advisory BaseTestCase::get_advisory(const std::string & name) {
    // This is used for testing queries as well, hence we don't use the AdvisoryQuery facility for filtering
    libdnf5::advisory::AdvisorySet advisories = libdnf5::advisory::AdvisoryQuery(base);

    libdnf5::advisory::AdvisorySet found(base);
    for (auto advisory : advisories) {
        if (advisory.get_name() == name) {
            found.add(advisory);
        }
    }

    if (found.empty()) {
        CPPUNIT_FAIL(fmt::format("No advisory \"{}\" found. All pool advisories:{}", name, to_string(advisories)));
    } else if (found.size() > 1) {
        CPPUNIT_FAIL(fmt::format("More than one advisory matching \"{}\" found:{}", name, to_string(advisories)));
    }

    return *found.begin();
}


libdnf5::comps::Environment BaseTestCase::get_environment(const std::string & environmentid, bool installed) {
    // This is used for testing queries as well, hence we don't use the EnvironmentQuery facility for filtering
    libdnf5::Set<libdnf5::comps::Environment> environments = libdnf5::comps::EnvironmentQuery(base);

    std::set<libdnf5::comps::Environment> found;
    for (auto environment : environments) {
        if (environment.get_environmentid() == environmentid && environment.get_installed() == installed) {
            found.insert(environment);
        }
    }

    if (found.empty()) {
        CPPUNIT_FAIL(fmt::format(
            "No environment \"{}\" found. All pool environments:{}", environmentid, to_string(environments)));
    } else if (found.size() > 1) {
        CPPUNIT_FAIL(
            fmt::format("More than one environment matching \"{}\" found:{}", environmentid, to_string(environments)));
    }

    return *found.begin();
}


libdnf5::comps::Group BaseTestCase::get_group(const std::string & groupid, bool installed) {
    // This is used for testing queries as well, hence we don't use the GroupQuery facility for filtering
    libdnf5::Set<libdnf5::comps::Group> groups = libdnf5::comps::GroupQuery(base);

    std::set<libdnf5::comps::Group> found;
    for (auto group : groups) {
        if (group.get_groupid() == groupid && group.get_installed() == installed) {
            found.insert(group);
        }
    }

    if (found.empty()) {
        CPPUNIT_FAIL(fmt::format("No group \"{}\" found. All pool groups:{}", groupid, to_string(groups)));
    } else if (found.size() > 1) {
        CPPUNIT_FAIL(fmt::format("More than one group matching \"{}\" found:{}", groupid, to_string(groups)));
    }

    return *found.begin();
}


libdnf5::rpm::Package BaseTestCase::get_pkg(const std::string & nevra, bool installed) {
    libdnf5::rpm::PackageQuery query(base);
    query.filter_nevra({nevra});
    if (installed) {
        query.filter_installed();
    } else {
        query.filter_available();
    }
    return first_query_pkg(query, nevra + " (" + (installed ? "installed" : "available") + ")");
}


libdnf5::rpm::Package BaseTestCase::get_pkg(const std::string & nevra, const char * repo) {
    libdnf5::rpm::PackageQuery query(base);
    query.filter_nevra({nevra});
    query.filter_repo_id(repo);
    return first_query_pkg(query, nevra + " (repo: " + repo + ")");
}


libdnf5::rpm::Package BaseTestCase::get_pkg_i(const std::string & nevra, size_t index) {
    libdnf5::rpm::PackageQuery query(base);
    query.filter_nevra({nevra});

    if (query.size() <= index) {
        CPPUNIT_FAIL(fmt::format(
            "Package index {} out of bounds for \"{}\", query packages:{}", index, nevra, to_string(query)));
    }

    auto it = query.begin();
    while (index-- > 0) {
        ++it;
    }

    return *it;
}


libdnf5::rpm::Package BaseTestCase::add_cmdline_pkg(const std::string & relative_path) {
    std::string path = PROJECT_BINARY_DIR "/test/data/" + relative_path;
    return repo_sack->add_cmdline_packages({path}).at(path);
}


libdnf5::rpm::Package BaseTestCase::first_query_pkg(libdnf5::rpm::PackageQuery & query, const std::string & what) {
    if (query.empty()) {
        CPPUNIT_FAIL(fmt::format(
            "No package \"{}\" found. All sack packages:{}", what, to_string(libdnf5::rpm::PackageQuery(base))));
    } else if (query.size() > 1) {
        CPPUNIT_FAIL(fmt::format("More than one package matching \"{}\" found:{}", what, to_string(query)));
    }

    return *query.begin();
}


void BaseTestCase::setUp() {
    TestCaseFixture::setUp();

    base.get_logger()->add_logger(std::make_unique<LoggerRedirector>(test_logger));

    // TODO we could use get_preconfigured_base() for this now, but that would
    // need changing the `base` member to a unique_ptr
    temp_dir = std::make_unique<libdnf5::utils::fs::TempDir>("libdnf5_unittest");
    std::filesystem::create_directory(temp_dir->get_path() / "installroot");

    base.get_config().get_installroot_option().set(temp_dir->get_path() / "installroot");
    base.get_config().get_cachedir_option().set(temp_dir->get_path() / "cache");
    base.get_config().get_optional_metadata_types_option().set(libdnf5::OPTIONAL_METADATA_TYPES);

    // Prevent loading libdnf5 plugins
    base.get_config().get_plugins_option().set(false);

    base.get_vars()->set("arch", "x86_64");

    base.setup();

    repo_sack = base.get_repo_sack();
    sack = base.get_rpm_package_sack();
}

void BaseTestCase::dump_debugdata() {
    repo_sack->dump_debugdata("debugdata");
}
