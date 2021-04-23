/*
Copyright (C) 2020-2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_solv_query.hpp"

#include "test/libdnf/utils.hpp"

#include "libdnf/rpm/package_set.hpp"
#include "libdnf/rpm/solv_query.hpp"

#include <filesystem>
#include <set>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(RpmSolvQueryTest);


// make constructor public so we can create Package instances in the tests
class TestPackage : public libdnf::rpm::Package {
public:
    TestPackage(libdnf::rpm::SolvSack * sack, libdnf::rpm::PackageId id) : libdnf::rpm::Package(sack, id) {}
};


void RpmSolvQueryTest::setUp() {
    RepoFixture::setUp();
    add_repo_solv("solv-repo1");
}


void RpmSolvQueryTest::test_size() {
    libdnf::rpm::SolvQuery query(sack);
    CPPUNIT_ASSERT_EQUAL(5LU, query.size());
}

void RpmSolvQueryTest::test_ifilter_latest() {
    add_repo_solv("solv-24pkgs");
    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    // also add 2 time the same package
    sack->add_cmdline_package(rpm_path, false);
    sack->add_cmdline_package(rpm_path, false);

    {
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_latest(1);
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
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_latest(2);
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
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_latest(-1);
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
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_latest(-23);
        std::vector<std::string> expected = {"pkg-0:1-1.noarch"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
}

void RpmSolvQueryTest::test_ifilter_name() {
    // packages with Name == "pkg"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_name({"pkg"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages with Name matching "pkg*" glob
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_name({"pkg*"}, libdnf::sack::QueryCmp::GLOB);

    expected = {
        "pkg-0:1.2-3.src",
        "pkg-0:1.2-3.x86_64",
        "pkg-libs-0:1.2-3.x86_64",
        "pkg-libs-1:1.2-4.x86_64",
        "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));

    // ---

    // packages with Name matching "p?g" glob
    libdnf::rpm::SolvQuery query3(sack);
    query3.ifilter_name({"p?g"}, libdnf::sack::QueryCmp::GLOB);

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query3));

    // ---

    // packages with Name != "pkg"
    libdnf::rpm::SolvQuery query4(sack);
    query4.ifilter_name({"pkg"}, libdnf::sack::QueryCmp::NEQ);

    expected = {"pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query4));

    // ---

    // packages with Name == "Pkg" - case insensitive match
    libdnf::rpm::SolvQuery query5(sack);
    query5.ifilter_name({"Pkg"}, libdnf::sack::QueryCmp::IEXACT);

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query5));

    // ---

    // packages with Name matching "P?g" glob - case insensitive match
    libdnf::rpm::SolvQuery query6(sack);
    std::vector<std::string> names_glob_icase{"cq?lib"};
    query6.ifilter_name({"P?g"}, libdnf::sack::QueryCmp::IGLOB);

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query6));

    // ---

    // packages with Name that contain "kg-l"
    libdnf::rpm::SolvQuery query7(sack);
    query7.ifilter_name({"kg-l"}, libdnf::sack::QueryCmp::CONTAINS);

    expected = {"pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query7));

    // ---

    // packages with Name that contain "kG-l" - case insensitive match
    libdnf::rpm::SolvQuery query8(sack);
    query8.ifilter_name({"kG-l"}, libdnf::sack::QueryCmp::ICONTAINS);

    expected = {"pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query8));

    // ---

    // unsupported comparison type (operator)
    CPPUNIT_ASSERT_THROW(
        query8.ifilter_name({"pkg"}, libdnf::sack::QueryCmp::GT), libdnf::rpm::SolvQuery::NotSupportedCmpType);

    // ---

    // packages with Name "pkg" or "pkg-libs" - two patterns matched in one expression
    libdnf::rpm::SolvQuery query9(sack);
    query9.ifilter_name({"pkg", "pkg-libs"});

    expected = {
        "pkg-0:1.2-3.src",
        "pkg-0:1.2-3.x86_64",
        "pkg-libs-0:1.2-3.x86_64",
        "pkg-libs-1:1.2-4.x86_64",
        "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query9));
}

void RpmSolvQueryTest::test_ifilter_name_packgset() {
    // packages with Name == "pkg"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_name({"pkg"});
    query1.ifilter_arch({"src"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_name(query1);

    std::vector<std::string> expected2 = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected2, to_vector_string(query2));
}

void RpmSolvQueryTest::test_ifilter_name_arch() {
    // packages with Name == "pkg"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_name({"pkg"});
    query1.ifilter_arch({"src"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_name_arch(query1);

    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}

void RpmSolvQueryTest::test_ifilter_nevra() {
    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra({"pkg-1.2-3.src", "pkg-1.2-3.x86_64"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra({"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - single argument
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra({"pkg-1.2-3.src"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument with 0 epoch - single argument
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra({"pkg-0:1.2-3.src"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument with unknown release - two elements
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra({"pkg-0:1.2-unknown.src", "pkg-0:1.2-unknown1.x86_64"});
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument with unknown version - single argument
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra({"pkg-0:1.2-unknown2.x86_64"});
        CPPUNIT_ASSERT_EQUAL(0LU, query.size());
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument without epoch, but package with epoch - single argument
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra({"pkg-libs-1.2-4.x86_64"});
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
}


void RpmSolvQueryTest::test_ifilter_version() {
    // packages with version == "1.2"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_version({"1.2"});

    std::vector<std::string> expected = {
        "pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages with version != "1.2"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_version({"1.2"}, libdnf::sack::QueryCmp::NEQ);

    expected = {"pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}


void RpmSolvQueryTest::test_ifilter_release() {
    // packages with release == "3"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_release({"3"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages with Release != "3"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_release({"3"}, libdnf::sack::QueryCmp::NEQ);

    expected = {"pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}

void RpmSolvQueryTest::test_ifilter_priority() {
    add_repo_solv("solv-24pkgs");
    add_repo_solv("solv-repo1");
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_priority();
    /// TODO(jmracek) Run test with repository with a different priority and check result
}

void RpmSolvQueryTest::test_ifilter_provides() {
    // packages with Provides == "libpkg.so.0()(64bit)"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_provides({"libpkg.so.0()(64bit)"});

    std::vector<std::string> expected = {"pkg-libs-1:1.2-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages without Provides == "libpkg.so.0()(64bit)"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_provides({"libpkg.so.0()(64bit)"}, libdnf::sack::QueryCmp::NEQ);

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}


void RpmSolvQueryTest::test_ifilter_requires() {
    // packages with Requires == "pkg-libs"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_requires({"pkg-libs"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages without Requires == "pkg-libs"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_requires({"pkg-libs"}, libdnf::sack::QueryCmp::NEQ);

    expected = {"pkg-0:1.2-3.src", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}


void RpmSolvQueryTest::test_ifilter_chain() {
    libdnf::rpm::SolvQuery query(sack);
    query.ifilter_name({"pkg"})
        .ifilter_epoch({"0"})
        .ifilter_version({"1.2"})
        .ifilter_release({"3"})
        .ifilter_arch({"x86_64"})
        .ifilter_provides({"foo"}, libdnf::sack::QueryCmp::NEQ)
        .ifilter_requires({"foo"}, libdnf::sack::QueryCmp::NEQ);

    std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
}


void RpmSolvQueryTest::test_resolve_pkg_spec() {
    {
        // test Name.Arch
        libdnf::rpm::SolvQuery query(sack);
        auto return_value = query.resolve_pkg_spec("pkg.x86_64", false, true, false, false, true, {});
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test NA icase
        libdnf::rpm::SolvQuery query(sack);
        auto return_value = query.resolve_pkg_spec("Pkg.x86_64", true, true, false, false, true, {});
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test a provide
        libdnf::rpm::SolvQuery query(sack);
        auto return_value = query.resolve_pkg_spec("pkg >= 1", false, true, true, false, true, {});
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test NEVRA glob
        libdnf::rpm::SolvQuery query(sack);
        auto return_value = query.resolve_pkg_spec("pk?-?:1.?-?.x8?_64", false, true, false, false, true, {});
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test NEVRA glob - icase == false, nothing found
        libdnf::rpm::SolvQuery query(sack);
        auto return_value = query.resolve_pkg_spec("Pk?-?:1.?-?.x8?_64", false, true, false, false, true, {});
        CPPUNIT_ASSERT_EQUAL(return_value.first, false);
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test NEVRA glob - icase == true
        libdnf::rpm::SolvQuery query(sack);
        auto return_value = query.resolve_pkg_spec("Pk?-?:1.?-?.x8?_64", true, true, false, false, true, {});
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test NEVRA icase
        libdnf::rpm::SolvQuery query(sack);
        auto return_value = query.resolve_pkg_spec("Pkg-0:1.2-3.X86_64", true, true, false, false, true, {});
        std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
}


void RpmSolvQueryTest::test_update() {
    // packages with Release == "3"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_release({"3"});

    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_name({"pkg-libs"});
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


void RpmSolvQueryTest::test_intersection() {
    // packages with Release == "3"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_release({"3"});
    CPPUNIT_ASSERT_EQUAL(3LU, query1.size());

    // packages with Name == "pkg-libs"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_name({"pkg-libs"});
    CPPUNIT_ASSERT_EQUAL(3LU, query2.size());

    query1.intersection(query2);
    CPPUNIT_ASSERT_EQUAL(1LU, query1.size());

    // check the resulting NEVRAs
    std::vector<std::string> expected = {"pkg-libs-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));
}


void RpmSolvQueryTest::test_difference() {
    // packages with Release == "3"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_release({"3"});
    CPPUNIT_ASSERT_EQUAL(3LU, query1.size());

    // packages with Release == "3" and name == "pkg-libs"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_release({"3"});
    query2.ifilter_name({"pkg-libs"});
    CPPUNIT_ASSERT_EQUAL(1LU, query2.size());

    query1.difference(query2);
    CPPUNIT_ASSERT_EQUAL(2LU, query1.size());

    // check the resulting NEVRAs
    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));
}
