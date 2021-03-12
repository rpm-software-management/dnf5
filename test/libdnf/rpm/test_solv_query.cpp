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

#include "libdnf/rpm/package_set.hpp"
#include "libdnf/rpm/solv_query.hpp"
#include "test/libdnf/utils.hpp"

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


void RpmSolvQueryTest::test_ifilter_name() {
    // packages with Name == "pkg"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_name(libdnf::sack::QueryCmp::EQ, {"pkg"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages with Name matching "pkg*" glob
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_name(libdnf::sack::QueryCmp::GLOB, {"pkg*"});

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));

    // ---

    // packages with Name matching "p?g" glob
    libdnf::rpm::SolvQuery query3(sack);
    query3.ifilter_name(libdnf::sack::QueryCmp::GLOB, {"p?g"});

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query3));

    // ---

    // packages with Name != "pkg"
    libdnf::rpm::SolvQuery query4(sack);
    query4.ifilter_name(libdnf::sack::QueryCmp::NEQ, {"pkg"});

    expected = {"pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query4));

    // ---

    // packages with Name == "Pkg" - case insensitive match
    libdnf::rpm::SolvQuery query5(sack);
    query5.ifilter_name(libdnf::sack::QueryCmp::IEXACT, {"Pkg"});

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query5));

    // ---

    // packages with Name matching "P?g" glob - case insensitive match
    libdnf::rpm::SolvQuery query6(sack);
    std::vector<std::string> names_glob_icase{"cq?lib"};
    query6.ifilter_name(libdnf::sack::QueryCmp::IGLOB, {"P?g"});

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query6));

    // ---

    // packages with Name that contain "kg-l"
    libdnf::rpm::SolvQuery query7(sack);
    query7.ifilter_name(libdnf::sack::QueryCmp::CONTAINS, {"kg-l"});

    expected = {"pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query7));

    // ---

    // packages with Name that contain "kG-l" - case insensitive match
    libdnf::rpm::SolvQuery query8(sack);
    query8.ifilter_name(libdnf::sack::QueryCmp::ICONTAINS, {"kG-l"});

    expected = {"pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query8));

    // ---

    // unsupported comparison type (operator)
    CPPUNIT_ASSERT_THROW(
        query8.ifilter_name(libdnf::sack::QueryCmp::GT, {"pkg"}),
        libdnf::rpm::SolvQuery::NotSupportedCmpType
    );

    // ---

    // packages with Name "pkg" or "pkg-libs" - two patterns matched in one expression
    libdnf::rpm::SolvQuery query9(sack);
    query9.ifilter_name(libdnf::sack::QueryCmp::EQ, {"pkg", "pkg-libs"});

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query9));
}


void RpmSolvQueryTest::test_ifilter_nevra() {
    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, {"pkg-1.2-3.src", "pkg-1.2-3.x86_64"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - single argument
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, {"pkg-1.2-3.src"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument with 0 epoch - single argument
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, {"pkg-0:1.2-3.src"});
        std::vector<std::string> expected = {"pkg-0:1.2-3.src"};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument with unknown release - two elements
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, {"pkg-0:1.2-unknown.src", "pkg-0:1.2-unknown1.x86_64"});
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument with unknown version - single argument
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, {"pkg-0:1.2-unknown2.x86_64"});
        CPPUNIT_ASSERT_EQUAL(0LU, query.size());
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }

    {
        // Test QueryCmp::EQ - argument without epoch, but package with epoch - single argument
        libdnf::rpm::SolvQuery query(sack);
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, {"pkg-libs-1.2-4.x86_64"});
        std::vector<std::string> expected = {};
        CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));
    }
}


void RpmSolvQueryTest::test_ifilter_version() {
    // packages with version == "1.2"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_version(libdnf::sack::QueryCmp::EQ, {"1.2"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages with version != "1.2"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_version(libdnf::sack::QueryCmp::NEQ, {"1.2"});

    expected = {"pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}


void RpmSolvQueryTest::test_ifilter_release() {
    // packages with release == "3"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_release(libdnf::sack::QueryCmp::EQ, {"3"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages with Release != "3"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_release(libdnf::sack::QueryCmp::NEQ, {"3"});

    expected = {"pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}


void RpmSolvQueryTest::test_ifilter_provides() {
    // packages with Provides == "libpkg.so.0()(64bit)"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_provides(libdnf::sack::QueryCmp::EQ, {"libpkg.so.0()(64bit)"});

    std::vector<std::string> expected = {"pkg-libs-1:1.2-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages without Provides == "libpkg.so.0()(64bit)"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_provides(libdnf::sack::QueryCmp::NEQ, {"libpkg.so.0()(64bit)"});

    expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}


void RpmSolvQueryTest::test_ifilter_requires() {
    // packages with Requires == "pkg-libs"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_requires(libdnf::sack::QueryCmp::EQ, {"pkg-libs"});

    std::vector<std::string> expected = {"pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));

    // ---

    // packages without Requires == "pkg-libs"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_requires(libdnf::sack::QueryCmp::NEQ, {"pkg-libs"});

    expected = {"pkg-0:1.2-3.src", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query2));
}


void RpmSolvQueryTest::test_ifilter_chain() {
    libdnf::rpm::SolvQuery query(sack);
    query \
        .ifilter_name(libdnf::sack::QueryCmp::EQ, {"pkg"}) \
        .ifilter_epoch(libdnf::sack::QueryCmp::EQ, {"0"}) \
        .ifilter_version(libdnf::sack::QueryCmp::EQ, {"1.2"}) \
        .ifilter_release(libdnf::sack::QueryCmp::EQ, {"3"}) \
        .ifilter_arch(libdnf::sack::QueryCmp::EQ, {"x86_64"}) \
        .ifilter_provides(libdnf::sack::QueryCmp::NEQ, {"foo"}) \
        .ifilter_requires(libdnf::sack::QueryCmp::NEQ, {"foo"});

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
    query1.ifilter_release(libdnf::sack::QueryCmp::EQ, {"3"});

    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_name(libdnf::sack::QueryCmp::EQ, {"pkg-libs"});
    CPPUNIT_ASSERT_EQUAL(3LU, query2.size());

    query1.update(query2);
    CPPUNIT_ASSERT_EQUAL(5LU, query1.size());

    // check the resulting NEVRAs
    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64", "pkg-libs-0:1.2-3.x86_64", "pkg-libs-1:1.2-4.x86_64", "pkg-libs-1:1.3-4.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));
}


void RpmSolvQueryTest::test_intersection() {
    // packages with Release == "3"
    libdnf::rpm::SolvQuery query1(sack);
    query1.ifilter_release(libdnf::sack::QueryCmp::EQ, {"3"});
    CPPUNIT_ASSERT_EQUAL(3LU, query1.size());

    // packages with Name == "pkg-libs"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_name(libdnf::sack::QueryCmp::EQ, {"pkg-libs"});
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
    query1.ifilter_release(libdnf::sack::QueryCmp::EQ, {"3"});
    CPPUNIT_ASSERT_EQUAL(3LU, query1.size());

    // packages with Release == "3" and name == "pkg-libs"
    libdnf::rpm::SolvQuery query2(sack);
    query2.ifilter_release(libdnf::sack::QueryCmp::EQ, {"3"});
    query2.ifilter_name(libdnf::sack::QueryCmp::EQ, {"pkg-libs"});
    CPPUNIT_ASSERT_EQUAL(1LU, query2.size());

    query1.difference(query2);
    CPPUNIT_ASSERT_EQUAL(2LU, query1.size());

    // check the resulting NEVRAs
    std::vector<std::string> expected = {"pkg-0:1.2-3.src", "pkg-0:1.2-3.x86_64"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query1));
}
