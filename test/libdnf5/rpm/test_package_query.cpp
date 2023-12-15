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

#include "../shared/utils.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>

#include <filesystem>
#include <set>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(RpmPackageQueryTest);

using namespace libdnf5::rpm;

namespace {

// make constructor public so we can create Package instances in the tests
class TestPackage : public Package {
public:
    TestPackage(const libdnf5::BaseWeakPtr & base, PackageId id) : libdnf5::rpm::Package(base, id) {}
};

}  // namespace


void RpmPackageQueryTest::setUp() {
    BaseTestCase::setUp();
}


void RpmPackageQueryTest::test_size() {
    add_repo_solv("solv-repo1");

    PackageQuery query(base);
    CPPUNIT_ASSERT_EQUAL((size_t)5, query.size());
}

void RpmPackageQueryTest::test_filter_latest_evr() {
    add_repo_solv("solv-repo1");
    add_repo_solv("solv-24pkgs");

    const std::string rpm_path = "cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    // also add 2 time the same package
    add_cmdline_pkg(rpm_path);
    add_cmdline_pkg(rpm_path);

    {
        PackageQuery query(base);
        query.filter_latest_evr(1);
        std::vector<Package> expected = {
            get_pkg("pkg-0:1.2-3.src"),
            get_pkg("pkg-0:1.2-3.x86_64"),
            get_pkg("pkg-libs-1:1.3-4.x86_64"),
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
            get_pkg("pkg-0:1.2-3.x86_64"),
            get_pkg("pkg-libs-1:1.2-4.x86_64"),
            get_pkg("pkg-libs-1:1.3-4.x86_64"),
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
            get_pkg("pkg-libs-0:1.2-3.x86_64"), get_pkg("pkg-libs-1:1.2-4.x86_64"), get_pkg("pkg-0:1-1.noarch"),
            get_pkg("pkg-0:1-2.noarch"),        get_pkg("pkg-0:1-3.noarch"),        get_pkg("pkg-0:1-4.noarch"),
            get_pkg("pkg-0:1-5.noarch"),        get_pkg("pkg-0:1-6.noarch"),        get_pkg("pkg-0:1-7.noarch"),
            get_pkg("pkg-0:1-8.noarch"),        get_pkg("pkg-0:1-9.noarch"),        get_pkg("pkg-0:1-10.noarch"),
            get_pkg("pkg-0:1-11.noarch"),       get_pkg("pkg-0:1-12.noarch"),       get_pkg("pkg-0:1-13.noarch"),
            get_pkg("pkg-0:1-14.noarch"),       get_pkg("pkg-0:1-15.noarch"),       get_pkg("pkg-0:1-16.noarch"),
            get_pkg("pkg-0:1-17.noarch"),       get_pkg("pkg-0:1-18.noarch"),       get_pkg("pkg-0:1-19.noarch"),
            get_pkg("pkg-0:1-20.noarch"),       get_pkg("pkg-0:1-21.noarch"),       get_pkg("pkg-0:1-22.noarch"),
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

void RpmPackageQueryTest::test_filter_latest_evr_ignore_arch() {
    add_repo_solv("solv-multiarch");

    {
        // Result of filter_latest_evr should include a package from each arch
        PackageQuery query(base);
        query.filter_latest_evr(1);
        std::vector<Package> expected = {
            get_pkg("foo-0:1.2-1.x86_64"),
            get_pkg("foo-0:1.2-2.noarch"),
            get_pkg("bar-0:4.5-1.noarch"),
            get_pkg("bar-0:4.5-2.x86_64"),
        };
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
    {
        // Result of filter_latest_evr_ignore_arch should include only the latest
        // packages, regardless of arch
        PackageQuery query(base);
        query.filter_latest_evr_any_arch(1);
        std::vector<Package> expected = {get_pkg("foo-0:1.2-2.noarch"), get_pkg("bar-0:4.5-2.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
}

void RpmPackageQueryTest::test_filter_earliest_evr() {
    add_repo_solv("solv-repo1");
    add_repo_solv("solv-24pkgs");

    std::string rpm_path = "cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    // also add 2 time the same package
    add_cmdline_pkg(rpm_path);
    add_cmdline_pkg(rpm_path);

    {
        PackageQuery query(base);
        query.filter_earliest_evr(1);
        std::vector<Package> expected = {
            get_pkg("pkg-0:1.2-3.src"),
            get_pkg("pkg-0:1.2-3.x86_64"),
            get_pkg("pkg-libs-0:1.2-3.x86_64"),
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
            get_pkg("pkg-0:1.2-3.x86_64"),
            get_pkg("pkg-libs-0:1.2-3.x86_64"),
            get_pkg("pkg-libs-1:1.2-4.x86_64"),
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
            get_pkg("pkg-libs-1:1.2-4.x86_64"), get_pkg("pkg-libs-1:1.3-4.x86_64"), get_pkg("pkg-0:1-2.noarch"),
            get_pkg("pkg-0:1-3.noarch"),        get_pkg("pkg-0:1-4.noarch"),        get_pkg("pkg-0:1-5.noarch"),
            get_pkg("pkg-0:1-6.noarch"),        get_pkg("pkg-0:1-7.noarch"),        get_pkg("pkg-0:1-8.noarch"),
            get_pkg("pkg-0:1-9.noarch"),        get_pkg("pkg-0:1-10.noarch"),       get_pkg("pkg-0:1-11.noarch"),
            get_pkg("pkg-0:1-12.noarch"),       get_pkg("pkg-0:1-13.noarch"),       get_pkg("pkg-0:1-14.noarch"),
            get_pkg("pkg-0:1-15.noarch"),       get_pkg("pkg-0:1-16.noarch"),       get_pkg("pkg-0:1-17.noarch"),
            get_pkg("pkg-0:1-18.noarch"),       get_pkg("pkg-0:1-19.noarch"),       get_pkg("pkg-0:1-20.noarch"),
            get_pkg("pkg-0:1-21.noarch"),       get_pkg("pkg-0:1-22.noarch"),       get_pkg("pkg-0:1-23.noarch"),
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

void RpmPackageQueryTest::test_filter_earliest_evr_ignore_arch() {
    add_repo_solv("solv-multiarch");

    {
        // Result of filter_earliest_evr should include a package from each arch
        PackageQuery query(base);
        query.filter_earliest_evr(1);
        std::vector<Package> expected = {
            get_pkg("foo-0:1.2-1.x86_64"),
            get_pkg("foo-0:1.2-2.noarch"),
            get_pkg("bar-0:4.5-1.noarch"),
            get_pkg("bar-0:4.5-2.x86_64"),
        };
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
    {
        // Result of filter_earliest_evr_ignore_arch should include only the earliest
        // packages, regardless of arch
        PackageQuery query(base);
        query.filter_earliest_evr_any_arch(1);
        std::vector<Package> expected = {get_pkg("foo-0:1.2-1.x86_64"), get_pkg("bar-0:4.5-1.noarch")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
}

void RpmPackageQueryTest::test_filter_name() {
    add_repo_solv("solv-repo1");

    // packages with Name == "pkg"
    PackageQuery query1(base);
    query1.filter_name("pkg");

    std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg("pkg-0:1.2-3.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    // ---

    // packages with Name matching "pkg*" glob
    PackageQuery query2(base);
    query2.filter_name("pkg*", libdnf5::sack::QueryCmp::GLOB);

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg("pkg-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-1:1.2-4.x86_64"),
        get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));

    // ---

    // packages with Name matching "p?g" glob
    PackageQuery query3(base);
    query3.filter_name("p?g", libdnf5::sack::QueryCmp::GLOB);

    expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg("pkg-0:1.2-3.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query3));

    // ---

    // packages with Name != "pkg"
    PackageQuery query4(base);
    query4.filter_name("pkg", libdnf5::sack::QueryCmp::NEQ);

    expected = {
        get_pkg("pkg-libs-0:1.2-3.x86_64"), get_pkg("pkg-libs-1:1.2-4.x86_64"), get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query4));

    // ---

    // packages with Name == "Pkg" - case insensitive match
    PackageQuery query5(base);
    query5.filter_name("Pkg", libdnf5::sack::QueryCmp::IEXACT);

    expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg("pkg-0:1.2-3.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query5));

    // ---

    // packages with Name matching "P?g" glob - case insensitive match
    PackageQuery query6(base);
    std::vector<std::string> names_glob_icase{"cq?lib"};
    query6.filter_name("P?g", libdnf5::sack::QueryCmp::IGLOB);

    expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg("pkg-0:1.2-3.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query6));

    // ---

    // packages with Name that contain "kg-l"
    PackageQuery query7(base);
    query7.filter_name("kg-l", libdnf5::sack::QueryCmp::CONTAINS);

    expected = {
        get_pkg("pkg-libs-0:1.2-3.x86_64"), get_pkg("pkg-libs-1:1.2-4.x86_64"), get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query7));

    // ---

    // packages with Name that contain "kG-l" - case insensitive match
    PackageQuery query8(base);
    query8.filter_name("kG-l", libdnf5::sack::QueryCmp::ICONTAINS);

    expected = {
        get_pkg("pkg-libs-0:1.2-3.x86_64"), get_pkg("pkg-libs-1:1.2-4.x86_64"), get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query8));

    // ---

    // unsupported comparison type (operator)
    CPPUNIT_ASSERT_THROW(query8.filter_name("pkg", libdnf5::sack::QueryCmp::GT), libdnf5::AssertionError);

    // ---

    // packages with Name "pkg" or "pkg-libs" - two patterns matched in one expression
    PackageQuery query9(base);
    query9.filter_name(std::vector<std::string>{"pkg", "pkg-libs"});

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg("pkg-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-1:1.2-4.x86_64"),
        get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query9));
}

void RpmPackageQueryTest::test_filter_name_packgset() {
    add_repo_solv("solv-repo1");

    // packages with Name == "pkg"
    PackageQuery query1(base);
    query1.filter_name("pkg");
    query1.filter_arch({"src"});

    std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    PackageQuery query2(base);
    query2.filter_name(query1);

    std::vector<Package> expected2 = {get_pkg("pkg-0:1.2-3.src"), get_pkg("pkg-0:1.2-3.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected2, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_nevra_packgset() {
    add_repo_solv("solv-repo1");

    std::string rpm_path = "cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    repo_sack->get_system_repo()->add_rpm_package(PROJECT_BINARY_DIR "/test/data/" + rpm_path, false);
    add_cmdline_pkg(rpm_path);

    PackageQuery query1(base);
    query1.filter_name("cmdline");
    std::vector<Package> expected1 = {get_pkg("cmdline-0:1.2-3.noarch", true), get_pkg("cmdline-0:1.2-3.noarch")};
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector(query1));
    query1.filter_installed();

    std::vector<Package> expected = {get_pkg("cmdline-0:1.2-3.noarch", true)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));
    CPPUNIT_ASSERT_EQUAL((size_t)1, query1.size());

    PackageQuery query2(base);
    query2.filter_nevra(query1);

    CPPUNIT_ASSERT_EQUAL((size_t)2, query2.size());
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_nevra_packgset_cmp() {
    add_repo_solv("solv-repo1");

    // prepare query to compare packages with
    PackageQuery patterns(base);
    patterns.filter_nevra({"pkg-libs-1:1.2-4.x86_64"});
    std::vector<Package> expected = {get_pkg("pkg-libs-1:1.2-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Pattern preparation failed", expected, to_vector(patterns));

    {
        // comparator EQ
        PackageQuery query(base);
        query.filter_nevra(patterns, libdnf5::sack::QueryCmp::EQ);
        std::vector<Package> expected = {get_pkg("pkg-libs-1:1.2-4.x86_64")};
        CPPUNIT_ASSERT_EQUAL_MESSAGE("EQ comparator failed", expected, to_vector(query));
    }

    {
        // comparator NEQ
        PackageQuery query(base);
        query.filter_name("pkg-libs");
        query.filter_nevra(patterns, libdnf5::sack::QueryCmp::NEQ);
        std::vector<Package> expected = {get_pkg("pkg-libs-0:1.2-3.x86_64"), get_pkg("pkg-libs-1:1.3-4.x86_64")};
        CPPUNIT_ASSERT_EQUAL_MESSAGE("NEQ comparator failed", expected, to_vector(query));
    }

    {
        // comparator LT
        PackageQuery query(base);
        query.filter_nevra(patterns, libdnf5::sack::QueryCmp::LT);
        std::vector<Package> expected = {get_pkg("pkg-libs-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL_MESSAGE("LT comparator failed", expected, to_vector(query));
    }

    {
        // comparator LTE
        PackageQuery query(base);
        query.filter_nevra(patterns, libdnf5::sack::QueryCmp::LTE);
        std::vector<Package> expected = {get_pkg("pkg-libs-0:1.2-3.x86_64"), get_pkg("pkg-libs-1:1.2-4.x86_64")};
        CPPUNIT_ASSERT_EQUAL_MESSAGE("LTE comparator failed", expected, to_vector(query));
    }

    {
        // comparator GT
        PackageQuery query(base);
        query.filter_nevra(patterns, libdnf5::sack::QueryCmp::GT);
        std::vector<Package> expected = {get_pkg("pkg-libs-1:1.3-4.x86_64")};
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GT comparator failed", expected, to_vector(query));
    }

    {
        // comparator GTE
        PackageQuery query(base);
        query.filter_nevra(patterns, libdnf5::sack::QueryCmp::GTE);
        std::vector<Package> expected = {get_pkg("pkg-libs-1:1.2-4.x86_64"), get_pkg("pkg-libs-1:1.3-4.x86_64")};
        CPPUNIT_ASSERT_EQUAL_MESSAGE("GTE comparator failed", expected, to_vector(query));
    }
}

void RpmPackageQueryTest::test_filter_name_arch() {
    add_repo_solv("solv-repo1");

    // packages with Name == "pkg"
    PackageQuery query1(base);
    query1.filter_name("pkg");
    query1.filter_arch({"src"});

    std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    PackageQuery query2(base);
    query2.filter_name_arch(query1);

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_name_arch2() {
    add_repo_solv("solv-repo1");

    std::string rpm_path = "cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    repo_sack->get_system_repo()->add_rpm_package(PROJECT_BINARY_DIR "/test/data/" + rpm_path, false);
    add_cmdline_pkg(rpm_path);

    PackageQuery query1(base);
    query1.filter_name("cmdline");
    std::vector<Package> expected1 = {get_pkg("cmdline-0:1.2-3.noarch", true), get_pkg("cmdline-0:1.2-3.noarch")};
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector(query1));

    query1.filter_installed();
    std::vector<Package> expected = {get_pkg("cmdline-0:1.2-3.noarch", true)};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));
    CPPUNIT_ASSERT_EQUAL((size_t)1, query1.size());

    PackageQuery query2(base);
    query2.filter_name_arch(query1);

    CPPUNIT_ASSERT_EQUAL((size_t)2, query2.size());
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_nevra() {
    add_repo_solv("solv-repo1");

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        PackageQuery query(base);
        query.filter_nevra({"pkg-1.2-3.src", "pkg-1.2-3.x86_64"});
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg("pkg-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        PackageQuery query(base);
        query.filter_nevra({"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"});
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg("pkg-0:1.2-3.x86_64")};
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
        query.filter_nevra({"pkg-0:1.2-unknown.src", "pkg-0:1.2-unknown1.x86_64"});
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ - argument with unknown version - single argument
        PackageQuery query(base);
        query.filter_nevra({"pkg-0:1.2-unknown2.x86_64"});
        CPPUNIT_ASSERT_EQUAL((size_t)0, query.size());
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ - argument without epoch, but package with epoch - single argument
        PackageQuery query(base);
        query.filter_nevra({"pkg-libs-1.2-4.x86_64"});
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
}


void RpmPackageQueryTest::test_filter_version() {
    add_repo_solv("solv-repo1");

    // packages with version == "1.2"
    PackageQuery query1(base);
    query1.filter_version("1.2");

    std::vector<Package> expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg("pkg-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-1:1.2-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    // ---

    // packages with version != "1.2"
    PackageQuery query2(base);
    query2.filter_version("1.2", libdnf5::sack::QueryCmp::NEQ);

    expected = {get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));

    // packages with version < "1.3"
    PackageQuery query3(base);
    query3.filter_version({"1.3"}, libdnf5::sack::QueryCmp::LT);

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg("pkg-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-1:1.2-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query3));

    // packages with version <= "1.3"
    PackageQuery query4(base);
    query4.filter_version({"1.3"}, libdnf5::sack::QueryCmp::LTE);

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg("pkg-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-1:1.2-4.x86_64"),
        get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query4));
}


void RpmPackageQueryTest::test_filter_release() {
    add_repo_solv("solv-repo1");

    // packages with release == "3"
    PackageQuery query1(base);
    query1.filter_release("3");

    std::vector<Package> expected = {
        get_pkg("pkg-0:1.2-3.src"), get_pkg("pkg-0:1.2-3.x86_64"), get_pkg("pkg-libs-0:1.2-3.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    // ---

    // packages with Release != "3"
    PackageQuery query2(base);
    query2.filter_release("3", libdnf5::sack::QueryCmp::NEQ);

    expected = {get_pkg("pkg-libs-1:1.2-4.x86_64"), get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));

    // packages with release > "3"
    PackageQuery query3(base);
    query3.filter_release({"3"}, libdnf5::sack::QueryCmp::GT);

    expected = {get_pkg("pkg-libs-1:1.2-4.x86_64"), get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query3));

    // packages with release >= "3"
    PackageQuery query4(base);
    query4.filter_release({"3"}, libdnf5::sack::QueryCmp::GTE);

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg("pkg-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-1:1.2-4.x86_64"),
        get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query4));
}

void RpmPackageQueryTest::test_filter_priority() {
    add_repo_solv("solv-repo1");
    add_repo_solv("solv-24pkgs");

    PackageQuery query1(base);
    query1.filter_priority();
    /// TODO(jmracek) Run test with repository with a different priority and check result
}

void RpmPackageQueryTest::test_filter_provides() {
    add_repo_solv("solv-repo1");

    // packages with Provides == "libpkg.so.0()(64bit)"
    PackageQuery query1(base);
    query1.filter_provides("libpkg.so.0()(64bit)");

    std::vector<Package> expected = {get_pkg("pkg-libs-1:1.2-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    // ---

    // packages without Provides == "libpkg.so.0()(64bit)"
    PackageQuery query2(base);
    query2.filter_provides("libpkg.so.0()(64bit)", libdnf5::sack::QueryCmp::NEQ);

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg("pkg-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));
}


void RpmPackageQueryTest::test_filter_requires() {
    add_repo_solv("solv-repo1");

    // packages with Requires == "pkg-libs"
    PackageQuery query1(base);
    query1.filter_requires({"pkg-libs"});

    std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));

    // ---

    // packages without Requires == "pkg-libs"
    PackageQuery query2(base);
    query2.filter_requires({"pkg-libs"}, libdnf5::sack::QueryCmp::NEQ);

    expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg("pkg-libs-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-1:1.2-4.x86_64"),
        get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query2));
}

void RpmPackageQueryTest::test_filter_advisories() {
    add_repo_repomd("repomd-repo1");

    {
        // Test QueryCmp::EQ with equal advisory pkg
        libdnf5::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("DNF-2019-1");
        libdnf5::rpm::PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf5::sack::QueryCmp::EQ);
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::GT with older advisory pkg
        libdnf5::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("PKG-OLDER");
        PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf5::sack::QueryCmp::GT);
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::LTE with older advisory pkg
        libdnf5::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("PKG-OLDER");
        PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf5::sack::QueryCmp::LTE);
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::LT with newer advisory pkg
        libdnf5::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("PKG-NEWER");
        PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf5::sack::QueryCmp::LT);
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::GTE with newer advisory pkg
        libdnf5::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("PKG-NEWER");
        PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf5::sack::QueryCmp::GTE);
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test QueryCmp::EQ with older and newer advisory pkg
        libdnf5::advisory::AdvisoryQuery adv_query(base);
        adv_query.filter_name("PKG-*", libdnf5::sack::QueryCmp::IGLOB);
        ;
        PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf5::sack::QueryCmp::EQ);
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
}

void RpmPackageQueryTest::test_filter_chain() {
    add_repo_solv("solv-repo1");

    PackageQuery query(base);
    query.filter_name("pkg");
    query.filter_epoch("0");
    query.filter_version("1.2");
    query.filter_release("3");
    query.filter_arch({"x86_64"});
    query.filter_provides("foo", libdnf5::sack::QueryCmp::NEQ);
    query.filter_requires({"foo"}, libdnf5::sack::QueryCmp::NEQ);

    std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
}


void RpmPackageQueryTest::test_resolve_pkg_spec() {
    add_repo_solv("solv-repo1");

    {
        // test Name.Arch
        PackageQuery query(base);
        libdnf5::ResolveSpecSettings settings;
        settings.set_with_provides(false);
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        auto return_value = query.resolve_pkg_spec("pkg.x86_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test NA icase
        PackageQuery query(base);
        libdnf5::ResolveSpecSettings settings;
        settings.set_ignore_case(true);
        settings.set_with_provides(false);
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        auto return_value = query.resolve_pkg_spec("Pkg.x86_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test a provide
        PackageQuery query(base);
        libdnf5::ResolveSpecSettings settings;
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        auto return_value = query.resolve_pkg_spec("pkg >= 1", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test NEVRA glob
        PackageQuery query(base);
        libdnf5::ResolveSpecSettings settings;
        settings.set_with_provides(false);
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        auto return_value = query.resolve_pkg_spec("pk?-?:1.?-?.x8?_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test NEVRA glob - icase == false, nothing found
        PackageQuery query(base);
        libdnf5::ResolveSpecSettings settings;
        settings.set_with_provides(false);
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        auto return_value = query.resolve_pkg_spec("Pk?-?:1.?-?.x8?_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, false);
        std::vector<Package> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test NEVRA glob - icase == true
        PackageQuery query(base);
        libdnf5::ResolveSpecSettings settings;
        settings.set_ignore_case(true);
        settings.set_with_provides(false);
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        auto return_value = query.resolve_pkg_spec("Pk?-?:1.?-?.x8?_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }

    {
        // Test NEVRA icase
        PackageQuery query(base);
        libdnf5::ResolveSpecSettings settings;
        settings.set_ignore_case(true);
        settings.set_with_provides(false);
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        auto return_value = query.resolve_pkg_spec("Pkg-0:1.2-3.X86_64", settings, true);
        std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.x86_64")};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));
    }
}


void RpmPackageQueryTest::test_update() {
    add_repo_solv("solv-repo1");

    // packages with Release == "3"
    PackageQuery query1(base);
    query1.filter_release("3");

    PackageQuery query2(base);
    query2.filter_name("pkg-libs");
    CPPUNIT_ASSERT_EQUAL((size_t)3, query2.size());

    query1.update(query2);
    CPPUNIT_ASSERT_EQUAL((size_t)5, query1.size());

    // check the resulting NEVRAs
    std::vector<Package> expected = {
        get_pkg("pkg-0:1.2-3.src"),
        get_pkg("pkg-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-0:1.2-3.x86_64"),
        get_pkg("pkg-libs-1:1.2-4.x86_64"),
        get_pkg("pkg-libs-1:1.3-4.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));
}


void RpmPackageQueryTest::test_intersection() {
    add_repo_solv("solv-repo1");

    // packages with Release == "3"
    PackageQuery query1(base);
    query1.filter_release("3");
    CPPUNIT_ASSERT_EQUAL((size_t)3, query1.size());

    // packages with Name == "pkg-libs"
    PackageQuery query2(base);
    query2.filter_name("pkg-libs");
    CPPUNIT_ASSERT_EQUAL((size_t)3, query2.size());

    query1.intersection(query2);
    CPPUNIT_ASSERT_EQUAL((size_t)1, query1.size());

    // check the resulting NEVRAs
    std::vector<Package> expected = {get_pkg("pkg-libs-0:1.2-3.x86_64")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query1));
}


void RpmPackageQueryTest::test_difference() {
    add_repo_solv("solv-repo1");

    // packages with Release == "3"
    PackageQuery query1(base);
    query1.filter_release("3");
    CPPUNIT_ASSERT_EQUAL((size_t)3, query1.size());

    // packages with Release == "3" and name == "pkg-libs"
    PackageQuery query2(base);
    query2.filter_release("3");
    query2.filter_name("pkg-libs");
    CPPUNIT_ASSERT_EQUAL((size_t)1, query2.size());

    query1.difference(query2);
    CPPUNIT_ASSERT_EQUAL((size_t)2, query1.size());

    // check the resulting NEVRAs
    std::vector<Package> expected = {get_pkg("pkg-0:1.2-3.src"), get_pkg("pkg-0:1.2-3.x86_64")};
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
        query.filter_provides("prv-all");
    }
}
