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


#include "test_package_query.hpp"

#include "utils.hpp"

#include "libdnf/rpm/package_query.hpp"
#include "libdnf/rpm/package_set.hpp"

#include <filesystem>
#include <set>
#include <string>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(RpmPackageQueryTest);

using namespace libdnf::rpm;

namespace {

// make constructor public so we can create Package instances in the tests
class TestPackage : public Package {
public:
    TestPackage(const libdnf::BaseWeakPtr & base, PackageId id) : libdnf::rpm::Package(base, id) {}
};

}  // namespace


void RpmPackageQueryTest::setUp() {
    BaseTestCase::setUp();
}


void RpmPackageQueryTest::test_size() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    PackageQuery query(base);
    CPPUNIT_ASSERT_EQUAL(5LU, query.size());
}

void RpmPackageQueryTest::test_filter_latest_evr() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);
    add_repo_solv("solv-24pkgs");

    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    // also add 2 time the same package
    repo_sack->get_cmdline_repo()->add_rpm_package(rpm_path, false);
    repo_sack->get_cmdline_repo()->add_rpm_package(rpm_path, false);

    {
        PackageQuery query(base);
        query.filter_latest_evr(1);
        std::vector<Package> expected = {
            get_pkg("pkg-0:1.2-3.src"),
            get_pkg(std::string("pkg-0:1.2-3.") + test_arch),
            get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch),
            get_pkg("pkg-0:1-24.noarch"),
            get_pkg_i("cmdline-0:1.2-3.noarch", 0),
            get_pkg_i("cmdline-0:1.2-3.noarch", 1)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
    {
        PackageQuery query(base);
        query.filter_latest_evr(2);
        std::vector<Package> expected = {
            get_pkg("pkg-0:1.2-3.src"),
            get_pkg(std::string("pkg-0:1.2-3.") + test_arch),
            get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
            get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch),
            get_pkg("pkg-0:1-23.noarch"),
            get_pkg("pkg-0:1-24.noarch"),
            get_pkg_i("cmdline-0:1.2-3.noarch", 0),
            get_pkg_i("cmdline-0:1.2-3.noarch", 1)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
    {
        PackageQuery query(base);
        query.filter_latest_evr(-1);
        std::vector<Package> expected = {
            get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
            get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
            get_pkg("pkg-0:1-1.noarch"),
            get_pkg("pkg-0:1-2.noarch"),
            get_pkg("pkg-0:1-3.noarch"),
            get_pkg("pkg-0:1-4.noarch"),
            get_pkg("pkg-0:1-5.noarch"),
            get_pkg("pkg-0:1-6.noarch"),
            get_pkg("pkg-0:1-7.noarch"),
            get_pkg("pkg-0:1-8.noarch"),
            get_pkg("pkg-0:1-9.noarch"),
            get_pkg("pkg-0:1-10.noarch"),
            get_pkg("pkg-0:1-11.noarch"),
            get_pkg("pkg-0:1-12.noarch"),
            get_pkg("pkg-0:1-13.noarch"),
            get_pkg("pkg-0:1-14.noarch"),
            get_pkg("pkg-0:1-15.noarch"),
            get_pkg("pkg-0:1-16.noarch"),
            get_pkg("pkg-0:1-17.noarch"),
            get_pkg("pkg-0:1-18.noarch"),
            get_pkg("pkg-0:1-19.noarch"),
            get_pkg("pkg-0:1-20.noarch"),
            get_pkg("pkg-0:1-21.noarch"),
            get_pkg("pkg-0:1-22.noarch"),
            get_pkg("pkg-0:1-23.noarch")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
    {
        PackageQuery query(base);
        query.filter_latest_evr(-23);
        std::vector<Package> expected = {get_pkg("pkg-0:1-1.noarch")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
}

void RpmPackageQueryTest::test_filter_earliest_evr() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);
    add_repo_solv("solv-24pkgs");

    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    // also add 2 time the same package
    repo_sack->get_cmdline_repo()->add_rpm_package(rpm_path, false);
    repo_sack->get_cmdline_repo()->add_rpm_package(rpm_path, false);

    {
        PackageQuery query(base);
        query.filter_earliest_evr(1);
        std::vector<Package> expected = {
            get_pkg("pkg-0:1.2-3.src"),
            get_pkg(std::string("pkg-0:1.2-3.") + test_arch),
            get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
            get_pkg("pkg-0:1-1.noarch"),
            get_pkg_i("cmdline-0:1.2-3.noarch", 0),
            get_pkg_i("cmdline-0:1.2-3.noarch", 1)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
    {
        PackageQuery query(base);
        query.filter_earliest_evr(2);
        std::vector<Package> expected = {
            get_pkg("pkg-0:1.2-3.src"),
            get_pkg(std::string("pkg-0:1.2-3.") + test_arch),
            get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
            get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
            get_pkg("pkg-0:1-1.noarch"),
            get_pkg("pkg-0:1-2.noarch"),
            get_pkg_i("cmdline-0:1.2-3.noarch", 0),
            get_pkg_i("cmdline-0:1.2-3.noarch", 1)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
    {
        PackageQuery query(base);
        query.filter_earliest_evr(-1);
        std::vector<Package> expected = {
            get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
            get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch),
            get_pkg("pkg-0:1-2.noarch"),
            get_pkg("pkg-0:1-3.noarch"),
            get_pkg("pkg-0:1-4.noarch"),
            get_pkg("pkg-0:1-5.noarch"),
            get_pkg("pkg-0:1-6.noarch"),
            get_pkg("pkg-0:1-7.noarch"),
            get_pkg("pkg-0:1-8.noarch"),
            get_pkg("pkg-0:1-9.noarch"),
            get_pkg("pkg-0:1-10.noarch"),
            get_pkg("pkg-0:1-11.noarch"),
            get_pkg("pkg-0:1-12.noarch"),
            get_pkg("pkg-0:1-13.noarch"),
            get_pkg("pkg-0:1-14.noarch"),
            get_pkg("pkg-0:1-15.noarch"),
            get_pkg("pkg-0:1-16.noarch"),
            get_pkg("pkg-0:1-17.noarch"),
            get_pkg("pkg-0:1-18.noarch"),
            get_pkg("pkg-0:1-19.noarch"),
            get_pkg("pkg-0:1-20.noarch"),
            get_pkg("pkg-0:1-21.noarch"),
            get_pkg("pkg-0:1-22.noarch"),
            get_pkg("pkg-0:1-23.noarch"),
            get_pkg("pkg-0:1-24.noarch")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
    {
        PackageQuery query(base);
        query.filter_earliest_evr(-23);
        std::vector<Package> expected = {get_pkg("pkg-0:1-24.noarch")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
}

void RpmPackageQueryTest::test_filter_name() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    // packages with Name == "pkg"
    PackageQuery query1(base);
    query1.filter_name({"pkg"});

    std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    // ---

    // packages with Name matching "pkg*" glob
    PackageQuery query2(base);
    query2.filter_name({"pkg*"}, libdnf::sack::QueryCmp::GLOB);

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg(std::string("pkg-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));

    // ---

    // packages with Name matching "p?g" glob
    PackageQuery query3(base);
    query3.filter_name({"p?g"}, libdnf::sack::QueryCmp::GLOB);

    expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query3));

    // ---

    // packages with Name != "pkg"
    PackageQuery query4(base);
    query4.filter_name({"pkg"}, libdnf::sack::QueryCmp::NEQ);

    expected = {
        get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query4));

    // ---

    // packages with Name == "Pkg" - case insensitive match
    PackageQuery query5(base);
    query5.filter_name({"Pkg"}, libdnf::sack::QueryCmp::IEXACT);

    expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query5));

    // ---

    // packages with Name matching "P?g" glob - case insensitive match
    PackageQuery query6(base);
    std::vector<std::string> names_glob_icase{"cq?lib"};
    query6.filter_name({"P?g"}, libdnf::sack::QueryCmp::IGLOB);

    expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query6));

    // ---

    // packages with Name that contain "kg-l"
    PackageQuery query7(base);
    query7.filter_name({"kg-l"}, libdnf::sack::QueryCmp::CONTAINS);

    expected = {
        get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query7));

    // ---

    // packages with Name that contain "kG-l" - case insensitive match
    PackageQuery query8(base);
    query8.filter_name({"kG-l"}, libdnf::sack::QueryCmp::ICONTAINS);

    expected = {
        get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query8));

    // ---

    // unsupported comparison type (operator)
    CPPUNIT_ASSERT_THROW(query8.filter_name({"pkg"}, libdnf::sack::QueryCmp::GT), libdnf::AssertionError);

    // ---

    // packages with Name "pkg" or "pkg-libs" - two patterns matched in one expression
    PackageQuery query9(base);
    query9.filter_name({"pkg", "pkg-libs"});

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg(std::string("pkg-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query9));
}

void RpmPackageQueryTest::test_filter_name_packgset() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    // packages with Name == "pkg"
    PackageQuery query1(base);
    query1.filter_name({"pkg"});
    query1.filter_arch({"src"});

    std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    PackageQuery query2(base);
    query2.filter_name(query1);

    std::vector<Package> expected2 = {get_pkg("pkg-0:1.2-3.src"), get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected2, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_nevra_packgset() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    repo_sack->get_system_repo()->add_rpm_package(rpm_path, false);
    repo_sack->get_cmdline_repo()->add_rpm_package(rpm_path, false);

    PackageQuery query1(base);
    query1.filter_name({"cmdline"});
    std::vector<Package> expected1 = {get_pkg("cmdline-0:1.2-3.noarch", true), get_pkg("cmdline-0:1.2-3.noarch")};
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector(query1));
    query1.filter_installed();

    std::vector<Package> expected = {get_pkg("cmdline-0:1.2-3.noarch", true)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));
    CPPUNIT_ASSERT_EQUAL(1lu, query1.size());

    PackageQuery query2(base);
    query2.filter_nevra(query1);

    CPPUNIT_ASSERT_EQUAL(2lu, query2.size());
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_name_arch() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    // packages with Name == "pkg"
    PackageQuery query1(base);
    query1.filter_name({"pkg"});
    query1.filter_arch({"src"});

    std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    PackageQuery query2(base);
    query2.filter_name_arch(query1);

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_name_arch2() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    repo_sack->get_system_repo()->add_rpm_package(rpm_path, false);
    repo_sack->get_cmdline_repo()->add_rpm_package(rpm_path, false);

    PackageQuery query1(base);
    query1.filter_name({"cmdline"});
    std::vector<Package> expected1 = {get_pkg("cmdline-0:1.2-3.noarch", true), get_pkg("cmdline-0:1.2-3.noarch")};
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector(query1));

    query1.filter_installed();
    std::vector<Package> expected = {get_pkg("cmdline-0:1.2-3.noarch", true)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));
    CPPUNIT_ASSERT_EQUAL(1lu, query1.size());

    PackageQuery query2(base);
    query2.filter_name_arch(query1);

    CPPUNIT_ASSERT_EQUAL(2lu, query2.size());
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_nevra() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        PackageQuery query(base);
        query.filter_nevra({"pkg-1.2-3.src", std::string("pkg-1.2-3.") + test_arch});
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        PackageQuery query(base);
        query.filter_nevra({"pkg-0:1.2-3.src", std::string("pkg-0:1.2-3.") + test_arch});
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - single argument
        PackageQuery query(base);
        query.filter_nevra({"pkg-1.2-3.src"});
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ - argument with 0 epoch - single argument
        PackageQuery query(base);
        query.filter_nevra({"pkg-0:1.2-3.src"});
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ - argument with unknown release - two elements
        PackageQuery query(base);
        query.filter_nevra({"pkg-0:1.2-unknown.src", std::string("pkg-0:1.2-unknown1.") + test_arch});
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ - argument with unknown version - single argument
        PackageQuery query(base);
        query.filter_nevra({std::string("pkg-0:1.2-unknown2.") + test_arch});
        CPPUNIT_ASSERT_EQUAL(0LU, query.size());
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ - argument without epoch, but package with epoch - single argument
        PackageQuery query(base);
        query.filter_nevra({std::string("pkg-libs-1.2-4.") + test_arch});
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
}


void RpmPackageQueryTest::test_filter_version() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    // packages with version == "1.2"
    PackageQuery query1(base);
    query1.filter_version({"1.2"});

    std::vector<Package> expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg(std::string("pkg-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    // ---

    // packages with version != "1.2"
    PackageQuery query2(base);
    query2.filter_version({"1.2"}, libdnf::sack::QueryCmp::NEQ);

    expected = {get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));
}


void RpmPackageQueryTest::test_filter_release() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    // packages with release == "3"
    PackageQuery query1(base);
    query1.filter_release({"3"});

    std::vector<Package> expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg(std::string("pkg-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    // ---

    // packages with Release != "3"
    PackageQuery query2(base);
    query2.filter_release({"3"}, libdnf::sack::QueryCmp::NEQ);

    expected = {
        get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch), get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_priority() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);
    add_repo_solv("solv-24pkgs");

    PackageQuery query1(base);
    query1.filter_priority();
    /// TODO(jmracek) Run test with repository with a different priority and check result
}

void RpmPackageQueryTest::test_filter_provides() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    // packages with Provides == "libpkg.so.0()(64bit)"
    PackageQuery query1(base);
    query1.filter_provides({"libpkg.so.0()(64bit)"});

    std::vector<Package> expected = {get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    // ---

    // packages without Provides == "libpkg.so.0()(64bit)"
    PackageQuery query2(base);
    query2.filter_provides({"libpkg.so.0()(64bit)"}, libdnf::sack::QueryCmp::NEQ);

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg(std::string("pkg-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));
}


void RpmPackageQueryTest::test_filter_requires() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    // packages with Requires == "pkg-libs"
    PackageQuery query1(base);
    query1.filter_requires({"pkg-libs"});

    std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    // ---

    // packages without Requires == "pkg-libs"
    PackageQuery query2(base);
    query2.filter_requires({"pkg-libs"}, libdnf::sack::QueryCmp::NEQ);

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_advisories() {
    add_repo_repomd("repomd-repo1");

    {
        // Test QueryCmp::EQ with equal advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("DNF-2019-1");
        libdnf::rpm::PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::EQ);
        std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::GT with older advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("PKG-OLDER");
        PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::GT);
        std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::LTE with older advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("PKG-OLDER");
        PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::LTE);
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::LT with newer advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("PKG-NEWER");
        PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::LT);
        std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::GTE with newer advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("PKG-NEWER");
        PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::GTE);
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ with older and newer advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("PKG-*", libdnf::sack::QueryCmp::IGLOB);
        ;
        PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::EQ);
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
}

void RpmPackageQueryTest::test_filter_chain() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    PackageQuery query(base);
    query.filter_name({"pkg"});
    query.filter_epoch({"0"});
    query.filter_version({"1.2"});
    query.filter_release({"3"});
    query.filter_arch({test_arch});
    query.filter_provides({"foo"}, libdnf::sack::QueryCmp::NEQ);
    query.filter_requires({"foo"}, libdnf::sack::QueryCmp::NEQ);

    std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
}


void RpmPackageQueryTest::test_resolve_pkg_spec() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    {
        // test Name.Arch
        PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec(std::string("pkg.") + test_arch, settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test NA icase
        PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec(std::string("Pkg.") + test_arch, settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test a provide
        PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.with_filenames = false};
        auto return_value = query.resolve_pkg_spec("pkg >= 1", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test NEVRA glob
        PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec("pk?-?:1.?-?.x8?_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test NEVRA glob - icase == false, nothing found
        PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec("Pk?-?:1.?-?.x8?_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, false);
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test NEVRA glob - icase == true
        PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec("Pk?-?:1.?-?.x8?_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test NEVRA icase
        PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec("Pkg-0:1.2-3.X86_64", settings, true);
        std::vector<Package> expected = {get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
}


void RpmPackageQueryTest::test_update() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    // packages with Release == "3"
    PackageQuery query1(base);
    query1.filter_release({"3"});

    PackageQuery query2(base);
    query2.filter_name({"pkg-libs"});
    CPPUNIT_ASSERT_EQUAL(3LU, query2.size());

    query1.update(query2);
    CPPUNIT_ASSERT_EQUAL(5LU, query1.size());

    // check the resulting NEVRAs
    std::vector<Package> expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg(std::string("pkg-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.2-4.") + test_arch),
        get_pkg(std::string("pkg-libs-1:1.3-4.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));
}


void RpmPackageQueryTest::test_intersection() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    // packages with Release == "3"
    PackageQuery query1(base);
    query1.filter_release({"3"});
    CPPUNIT_ASSERT_EQUAL(3LU, query1.size());

    // packages with Name == "pkg-libs"
    PackageQuery query2(base);
    query2.filter_name({"pkg-libs"});
    CPPUNIT_ASSERT_EQUAL(3LU, query2.size());

    query1.intersection(query2);
    CPPUNIT_ASSERT_EQUAL(1LU, query1.size());

    // check the resulting NEVRAs
    std::vector<Package> expected = {get_pkg(std::string("pkg-libs-0:1.2-3.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));
}


void RpmPackageQueryTest::test_difference() {
    add_repo_solv(std::string("solv-repo1-") + test_arch);

    // packages with Release == "3"
    PackageQuery query1(base);
    query1.filter_release({"3"});
    CPPUNIT_ASSERT_EQUAL(3LU, query1.size());

    // packages with Release == "3" and name == "pkg-libs"
    PackageQuery query2(base);
    query2.filter_release({"3"});
    query2.filter_name({"pkg-libs"});
    CPPUNIT_ASSERT_EQUAL(1LU, query2.size());

    query1.difference(query2);
    CPPUNIT_ASSERT_EQUAL(2LU, query1.size());

    // check the resulting NEVRAs
    std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg(std::string("pkg-0:1.2-3.") + test_arch)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));
}


void RpmPackageQueryTest::test_filter_latest_evr_performance() {
    add_repo_solv("solv-humongous");

    for (int i = 0; i < 10000; ++i) {
        PackageQuery query(base);
        query.filter_latest_evr();
    }
}


void RpmPackageQueryTest::test_filter_provides_performance() {
    add_repo_solv("solv-humongous");

    for (int i = 0; i < 100000; ++i) {
        PackageQuery query(base);
        query.filter_provides({"prv-all"});
    }
}
