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
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(RpmPackageQueryTest);


namespace {

// make constructor public so we can create Package instances in the tests
class TestPackage : public libdnf::rpm::Package {
public:
    TestPackage(const libdnf::BaseWeakPtr & base, libdnf::rpm::PackageId id) : libdnf::rpm::Package(base, id) {}
};

}  // namespace


void RpmPackageQueryTest::setUp() {
    LibdnfTestCase::setUp();
}


void RpmPackageQueryTest::test_size() {
    add_repo_solv("solv-repo1");

    libdnf::rpm::PackageQuery query(base);
    CPPUNIT_ASSERT_EQUAL(5LU, query.size());
}

void RpmPackageQueryTest::test_filter_latest_evr() {
    add_repo_solv("solv-repo1");
    add_repo_solv("solv-24pkgs");

    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    // also add 2 time the same package
    sack->add_cmdline_package(rpm_path, false);
    sack->add_cmdline_package(rpm_path, false);

    {
        libdnf::rpm::PackageQuery query(base);
        query.filter_latest_evr(1);
        std::vector<std::string> expected = {
            "pkg-0:1.2-3.src",
            "pkg-0:1.2-3.x86_64",
            "pkg-libs-1:1.3-4.x86_64",
            "pkg-0:1-24.noarch",
            "cmdline-0:1.2-3.noarch",
            "cmdline-0:1.2-3.noarch"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
    {
        libdnf::rpm::PackageQuery query(base);
        query.filter_latest_evr(2);
        std::vector<std::string> expected = {
            "pkg-0:1.2-3.src",
            "pkg-0:1.2-3.x86_64",
            "pkg-libs-1:1.2-4.x86_64",
            "pkg-libs-1:1.3-4.x86_64",
            "pkg-0:1-23.noarch",
            "pkg-0:1-24.noarch",
            "cmdline-0:1.2-3.noarch",
            "cmdline-0:1.2-3.noarch"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
    {
        libdnf::rpm::PackageQuery query(base);
        query.filter_latest_evr(-1);
        std::vector<std::string> expected = {
            "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-0:1-1.noarch",  "pkg-0:1-2.noarch",
            "pkg-0:1-3.noarch",        "pkg-0:1-4.noarch",        "pkg-0:1-5.noarch",  "pkg-0:1-6.noarch",
            "pkg-0:1-7.noarch",        "pkg-0:1-8.noarch",        "pkg-0:1-9.noarch",  "pkg-0:1-10.noarch",
            "pkg-0:1-11.noarch",       "pkg-0:1-12.noarch",       "pkg-0:1-13.noarch", "pkg-0:1-14.noarch",
            "pkg-0:1-15.noarch",       "pkg-0:1-16.noarch",       "pkg-0:1-17.noarch", "pkg-0:1-18.noarch",
            "pkg-0:1-19.noarch",       "pkg-0:1-20.noarch",       "pkg-0:1-21.noarch", "pkg-0:1-22.noarch",
            "pkg-0:1-23.noarch"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
    {
        libdnf::rpm::PackageQuery query(base);
        query.filter_latest_evr(-23);
        std::vector<std::string> expected = {"pkg-0:1-1.noarch"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
}

void RpmPackageQueryTest::test_filter_name() {
    add_repo_solv("solv-repo1");

    // packages with Name == "pkg"
    libdnf::rpm::PackageQuery query1(base);
    query1.filter_name({"pkg"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages with Name matching "pkg*" glob
    libdnf::rpm::PackageQuery query2(base);
    query2.filter_name({"pkg*"}, libdnf::sack::QueryCmp::GLOB);

    expected = {
        "pkg-0:1.2-3.src",
        "pkg-0:1.2-3.x86_64",
        "pkg-libs-0:1.2-3.x86_64",
        "pkg-libs-1:1.2-4.x86_64",
        "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));

    // ---

    // packages with Name matching "p?g" glob
    libdnf::rpm::PackageQuery query3(base);
    query3.filter_name({"p?g"}, libdnf::sack::QueryCmp::GLOB);

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query3));

    // ---

    // packages with Name != "pkg"
    libdnf::rpm::PackageQuery query4(base);
    query4.filter_name({"pkg"}, libdnf::sack::QueryCmp::NEQ);

    expected = {"pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query4));

    // ---

    // packages with Name == "Pkg" - case insensitive match
    libdnf::rpm::PackageQuery query5(base);
    query5.filter_name({"Pkg"}, libdnf::sack::QueryCmp::IEXACT);

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query5));

    // ---

    // packages with Name matching "P?g" glob - case insensitive match
    libdnf::rpm::PackageQuery query6(base);
    std::vector<std::string> names_glob_icase{"cq?lib"};
    query6.filter_name({"P?g"}, libdnf::sack::QueryCmp::IGLOB);

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query6));

    // ---

    // packages with Name that contain "kg-l"
    libdnf::rpm::PackageQuery query7(base);
    query7.filter_name({"kg-l"}, libdnf::sack::QueryCmp::CONTAINS);

    expected = {"pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query7));

    // ---

    // packages with Name that contain "kG-l" - case insensitive match
    libdnf::rpm::PackageQuery query8(base);
    query8.filter_name({"kG-l"}, libdnf::sack::QueryCmp::ICONTAINS);

    expected = {"pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query8));

    // ---

    // unsupported comparison type (operator)
    CPPUNIT_ASSERT_THROW(query8.filter_name({"pkg"}, libdnf::sack::QueryCmp::GT), libdnf::AssertionError);

    // ---

    // packages with Name "pkg" or "pkg-libs" - two patterns matched in one expression
    libdnf::rpm::PackageQuery query9(base);
    query9.filter_name({"pkg", "pkg-libs"});

    expected = {
        "pkg-0:1.2-3.src",
        "pkg-0:1.2-3.x86_64",
        "pkg-libs-0:1.2-3.x86_64",
        "pkg-libs-1:1.2-4.x86_64",
        "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query9));
}

void RpmPackageQueryTest::test_filter_name_packgset() {
    add_repo_solv("solv-repo1");

    // packages with Name == "pkg"
    libdnf::rpm::PackageQuery query1(base);
    query1.filter_name({"pkg"});
    query1.filter_arch({"src"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    libdnf::rpm::PackageQuery query2(base);
    query2.filter_name(query1);

    std::vector<std::string> expected2 = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected2, to_vector_string(query2));
}

void RpmPackageQueryTest::test_filter_nevra_packgset() {
    add_repo_solv("solv-repo1");

    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    sack->add_system_package(rpm_path, false);
    sack->add_cmdline_package(rpm_path, false);

    libdnf::rpm::PackageQuery query1(base);
    query1.filter_name({"cmdline"});
    std::vector<std::string> expected1 = {"cmdline-0:1.2-3.noarch", "cmdline-0:1.2-3.noarch"};
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector_string(query1));
    query1.filter_installed();

    std::vector<std::string> expected = {"cmdline-0:1.2-3.noarch"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));
    CPPUNIT_ASSERT_EQUAL(1lu, query1.size());

    libdnf::rpm::PackageQuery query2(base);
    query2.filter_nevra(query1);

    CPPUNIT_ASSERT_EQUAL(2lu, query2.size());
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector_string(query2));
}

void RpmPackageQueryTest::test_filter_name_arch() {
    add_repo_solv("solv-repo1");

    // packages with Name == "pkg"
    libdnf::rpm::PackageQuery query1(base);
    query1.filter_name({"pkg"});
    query1.filter_arch({"src"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    libdnf::rpm::PackageQuery query2(base);
    query2.filter_name_arch(query1);

    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}

void RpmPackageQueryTest::test_filter_name_arch2() {
    add_repo_solv("solv-repo1");

    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    sack->add_system_package(rpm_path, false);
    sack->add_cmdline_package(rpm_path, false);

    libdnf::rpm::PackageQuery query1(base);
    query1.filter_name({"cmdline"});
    std::vector<std::string> expected1 = {"cmdline-0:1.2-3.noarch", "cmdline-0:1.2-3.noarch"};
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector_string(query1));
    query1.filter_installed();

    std::vector<std::string> expected = {"cmdline-0:1.2-3.noarch"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));
    CPPUNIT_ASSERT_EQUAL(1lu, query1.size());

    libdnf::rpm::PackageQuery query2(base);
    query2.filter_name_arch(query1);

    CPPUNIT_ASSERT_EQUAL(2lu, query2.size());
    CPPUNIT_ASSERT_EQUAL(expected1, to_vector_string(query2));
}

void RpmPackageQueryTest::test_filter_nevra() {
    add_repo_solv("solv-repo1");

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        libdnf::rpm::PackageQuery query(base);
        query.filter_nevra({"pkg-1.2-3.src", "pkg-1.2-3.x86_64"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        libdnf::rpm::PackageQuery query(base);
        query.filter_nevra({"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - single argument
        libdnf::rpm::PackageQuery query(base);
        query.filter_nevra({"pkg-1.2-3.src"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument with 0 epoch - single argument
        libdnf::rpm::PackageQuery query(base);
        query.filter_nevra({"pkg-0:1.2-3.src"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument with unknown release - two elements
        libdnf::rpm::PackageQuery query(base);
        query.filter_nevra({"pkg-0:1.2-unknown.src", "pkg-0:1.2-unknown1.x86_64"});
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument with unknown version - single argument
        libdnf::rpm::PackageQuery query(base);
        query.filter_nevra({"pkg-0:1.2-unknown2.x86_64"});
        CPPUNIT_ASSERT_EQUAL(0LU, query.size());
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument without epoch, but package with epoch - single argument
        libdnf::rpm::PackageQuery query(base);
        query.filter_nevra({"pkg-libs-1.2-4.x86_64"});
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
}


void RpmPackageQueryTest::test_filter_version() {
    add_repo_solv("solv-repo1");

    // packages with version == "1.2"
    libdnf::rpm::PackageQuery query1(base);
    query1.filter_version({"1.2"});

    std::vector<std::string> expected = {
        "pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages with version != "1.2"
    libdnf::rpm::PackageQuery query2(base);
    query2.filter_version({"1.2"}, libdnf::sack::QueryCmp::NEQ);

    expected = {"pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}


void RpmPackageQueryTest::test_filter_release() {
    add_repo_solv("solv-repo1");

    // packages with release == "3"
    libdnf::rpm::PackageQuery query1(base);
    query1.filter_release({"3"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages with Release != "3"
    libdnf::rpm::PackageQuery query2(base);
    query2.filter_release({"3"}, libdnf::sack::QueryCmp::NEQ);

    expected = {"pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}

void RpmPackageQueryTest::test_filter_priority() {
    add_repo_solv("solv-repo1");
    add_repo_solv("solv-24pkgs");

    libdnf::rpm::PackageQuery query1(base);
    query1.filter_priority();
    /// TODO(jmracek) Run test with repository with a different priority and check result
}

void RpmPackageQueryTest::test_filter_provides() {
    add_repo_solv("solv-repo1");

    // packages with Provides == "libpkg.so.0()(64bit)"
    libdnf::rpm::PackageQuery query1(base);
    query1.filter_provides({"libpkg.so.0()(64bit)"});

    std::vector<std::string> expected = {"pkg-libs-1:1.2-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages without Provides == "libpkg.so.0()(64bit)"
    libdnf::rpm::PackageQuery query2(base);
    query2.filter_provides({"libpkg.so.0()(64bit)"}, libdnf::sack::QueryCmp::NEQ);

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}


void RpmPackageQueryTest::test_filter_requires() {
    add_repo_solv("solv-repo1");

    // packages with Requires == "pkg-libs"
    libdnf::rpm::PackageQuery query1(base);
    query1.filter_requires({"pkg-libs"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages without Requires == "pkg-libs"
    libdnf::rpm::PackageQuery query2(base);
    query2.filter_requires({"pkg-libs"}, libdnf::sack::QueryCmp::NEQ);

    expected = {"pkg-0:1.2-3.src", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}

void RpmPackageQueryTest::test_filter_advisories() {
    add_repo_repomd("repomd-repo1");

    {
        // Test QueryCmp::EQ with equal advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query = libdnf::advisory::AdvisoryQuery(base).filter_name("DNF-2019-1");
        libdnf::rpm::PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::EQ);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::GT with older advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query = libdnf::advisory::AdvisoryQuery(base).filter_name("PKG-OLDER");
        libdnf::rpm::PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::GT);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::LTE with older advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query = libdnf::advisory::AdvisoryQuery(base).filter_name("PKG-OLDER");
        libdnf::rpm::PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::LTE);
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::LT with newer advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query = libdnf::advisory::AdvisoryQuery(base).filter_name("PKG-NEWER");
        libdnf::rpm::PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::LT);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::GTE with newer advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query = libdnf::advisory::AdvisoryQuery(base).filter_name("PKG-NEWER");
        libdnf::rpm::PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::GTE);
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ with older and newer advisory pkg
        libdnf::advisory::AdvisoryQuery adv_query =
            libdnf::advisory::AdvisoryQuery(base).filter_name("PKG-*", libdnf::sack::QueryCmp::IGLOB);
        ;
        libdnf::rpm::PackageQuery query(base);
        query.filter_advisories(adv_query, libdnf::sack::QueryCmp::EQ);
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
}

void RpmPackageQueryTest::test_filter_chain() {
    add_repo_solv("solv-repo1");

    libdnf::rpm::PackageQuery query(base);
    query.filter_name({"pkg"})
        .filter_epoch({"0"})
        .filter_version({"1.2"})
        .filter_release({"3"})
        .filter_arch({"x86_64"})
        .filter_provides({"foo"}, libdnf::sack::QueryCmp::NEQ)
        .filter_requires({"foo"}, libdnf::sack::QueryCmp::NEQ);

    std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
}


void RpmPackageQueryTest::test_resolve_pkg_spec() {
    add_repo_solv("solv-repo1");

    {
        // test Name.Arch
        libdnf::rpm::PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec("pkg.x86_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test NA icase
        libdnf::rpm::PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec("Pkg.x86_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test a provide
        libdnf::rpm::PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.with_filenames = false};
        auto return_value = query.resolve_pkg_spec("pkg >= 1", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test NEVRA glob
        libdnf::rpm::PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec("pk?-?:1.?-?.x8?_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test NEVRA glob - icase == false, nothing found
        libdnf::rpm::PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec("Pk?-?:1.?-?.x8?_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, false);
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test NEVRA glob - icase == true
        libdnf::rpm::PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec("Pk?-?:1.?-?.x8?_64", settings, true);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test NEVRA icase
        libdnf::rpm::PackageQuery query(base);
        libdnf::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false, .with_filenames = false};
        auto return_value = query.resolve_pkg_spec("Pkg-0:1.2-3.X86_64", settings, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
}


void RpmPackageQueryTest::test_update() {
    add_repo_solv("solv-repo1");

    // packages with Release == "3"
    libdnf::rpm::PackageQuery query1(base);
    query1.filter_release({"3"});

    libdnf::rpm::PackageQuery query2(base);
    query2.filter_name({"pkg-libs"});
    CPPUNIT_ASSERT_EQUAL(3LU, query2.size());

    query1.update(query2);
    CPPUNIT_ASSERT_EQUAL(5LU, query1.size());

    // check the resulting NEVRAs
    std::vector<std::string> expected = {
        "pkg-0:1.2-3.src",
        "pkg-0:1.2-3.x86_64",
        "pkg-libs-0:1.2-3.x86_64",
        "pkg-libs-1:1.2-4.x86_64",
        "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));
}


void RpmPackageQueryTest::test_intersection() {
    add_repo_solv("solv-repo1");

    // packages with Release == "3"
    libdnf::rpm::PackageQuery query1(base);
    query1.filter_release({"3"});
    CPPUNIT_ASSERT_EQUAL(3LU, query1.size());

    // packages with Name == "pkg-libs"
    libdnf::rpm::PackageQuery query2(base);
    query2.filter_name({"pkg-libs"});
    CPPUNIT_ASSERT_EQUAL(3LU, query2.size());

    query1.intersection(query2);
    CPPUNIT_ASSERT_EQUAL(1LU, query1.size());

    // check the resulting NEVRAs
    std::vector<std::string> expected = {"pkg-libs-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));
}


void RpmPackageQueryTest::test_difference() {
    add_repo_solv("solv-repo1");

    // packages with Release == "3"
    libdnf::rpm::PackageQuery query1(base);
    query1.filter_release({"3"});
    CPPUNIT_ASSERT_EQUAL(3LU, query1.size());

    // packages with Release == "3" and name == "pkg-libs"
    libdnf::rpm::PackageQuery query2(base);
    query2.filter_release({"3"});
    query2.filter_name({"pkg-libs"});
    CPPUNIT_ASSERT_EQUAL(1LU, query2.size());

    query1.difference(query2);
    CPPUNIT_ASSERT_EQUAL(2LU, query1.size());

    // check the resulting NEVRAs
    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));
}


void RpmPackageQueryTest::test_filter_latest_evr_performance() {
    add_repo_solv("solv-humongous");

    for (int i = 0; i < 10000; ++i) {
        libdnf::rpm::PackageQuery query(base);
        query.filter_latest_evr();
    }
}


void RpmPackageQueryTest::test_filter_provides_performance() {
    add_repo_solv("solv-humongous");

    for (int i = 0; i < 100000; ++i) {
        libdnf::rpm::PackageQuery query(base);
        query.filter_provides({"prv-all"});
    }
}
