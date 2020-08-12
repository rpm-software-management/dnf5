/*
Copyright (C) 2020 Red Hat, Inc.

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
    add_repo("dnf-ci-fedora");
}

void RpmSolvQueryTest::test_size() {
    libdnf::rpm::SolvQuery query(sack.get());
    CPPUNIT_ASSERT_EQUAL(289lu, query.size());
}

void RpmSolvQueryTest::test_ifilter_name() {
    std::set<std::string> nevras{"CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64"};
    std::set<std::string> nevras_contains{"CQRlib-1.1.1-4.fc29.src",
                                          "CQRlib-1.1.1-4.fc29.x86_64",
                                          "CQRlib-devel-1.1.2-16.fc29.src",
                                          "CQRlib-devel-1.1.2-16.fc29.x86_64"};
    std::set<std::string> full_nevras{"CQRlib-0:1.1.1-4.fc29.src",
                                      "CQRlib-0:1.1.1-4.fc29.x86_64",
                                      "nodejs-1:5.12.1-1.fc29.src",
                                      "nodejs-1:5.12.1-1.fc29.x86_64"};

    // Test QueryCmp::EQ
    libdnf::rpm::SolvQuery query(sack.get());
    std::vector<std::string> names{"CQRlib"};
    query.ifilter_name(libdnf::sack::QueryCmp::EQ, names);
    CPPUNIT_ASSERT_EQUAL(2lu, query.size());
    auto pset = query.get_package_set();
    for (auto pkg : pset) {
        CPPUNIT_ASSERT(nevras.find(pkg.get_nevra()) != nevras.end());
    }

    // Test QueryCmp::GLOB
    libdnf::rpm::SolvQuery query2(sack.get());
    std::vector<std::string> names2{"CQ?lib"};
    query2.ifilter_name(libdnf::sack::QueryCmp::GLOB, names2);
    CPPUNIT_ASSERT_EQUAL(2lu, query2.size());
    auto pset2 = query2.get_package_set();
    for (auto pkg : pset2) {
        CPPUNIT_ASSERT(nevras.find(pkg.get_nevra()) != nevras.end());
    }

    // Test two filters ifilter_name().ifilter_arch()
    std::vector<std::string> arches{"src"};
    libdnf::rpm::SolvQuery query3(sack.get());
    query3.ifilter_name(libdnf::sack::QueryCmp::EQ, names).ifilter_arch(libdnf::sack::QueryCmp::EQ, arches);
    CPPUNIT_ASSERT_EQUAL(1lu, query3.size());
    auto pset3 = query3.get_package_set();
    for (auto pkg : pset3) {
        CPPUNIT_ASSERT(pkg.get_nevra() == "CQRlib-1.1.1-4.fc29.src");
    }

    // Test QueryCmp::NEQ
    libdnf::rpm::SolvQuery query4(sack.get());
    query4.ifilter_name(libdnf::sack::QueryCmp::NEQ, names);
    CPPUNIT_ASSERT_EQUAL(287lu, query4.size());

    // Test QueryCmp::IEXACT
    libdnf::rpm::SolvQuery query5(sack.get());
    std::vector<std::string> names_icase{"cqrlib"};
    query5.ifilter_name(libdnf::sack::QueryCmp::IEXACT, names_icase);
    CPPUNIT_ASSERT_EQUAL(2lu, query5.size());
    auto pset4 = query5.get_package_set();
    for (auto pkg : pset4) {
        CPPUNIT_ASSERT(nevras.find(pkg.get_nevra()) != nevras.end());
    }
    query5.ifilter_name(libdnf::sack::QueryCmp::EQ, names_icase);
    CPPUNIT_ASSERT_EQUAL(0lu, query5.size());

    // Test QueryCmp::IGLOB
    libdnf::rpm::SolvQuery query6(sack.get());
    std::vector<std::string> names_glob_icase{"cq?lib"};
    query6.ifilter_name(libdnf::sack::QueryCmp::IGLOB, names_glob_icase);
    CPPUNIT_ASSERT_EQUAL(2lu, query6.size());
    auto pset5 = query6.get_package_set();
    for (auto pkg : pset5) {
        CPPUNIT_ASSERT(nevras.find(pkg.get_nevra()) != nevras.end());
    }
    query6.ifilter_name(libdnf::sack::QueryCmp::GLOB, names_glob_icase);
    CPPUNIT_ASSERT_EQUAL(0lu, query6.size());

    // Test QueryCmp::CONTAINS
    libdnf::rpm::SolvQuery query7(sack.get());
    std::vector<std::string> names_contains{"QRli"};
    query7.ifilter_name(libdnf::sack::QueryCmp::CONTAINS, names_contains);
    CPPUNIT_ASSERT_EQUAL(4lu, query7.size());
    auto pset6 = query7.get_package_set();
    for (auto pkg : pset6) {
        CPPUNIT_ASSERT(nevras_contains.find(pkg.get_nevra()) != nevras_contains.end());
    }

    // Test QueryCmp::ICONTAINS
    libdnf::rpm::SolvQuery query8(sack.get());
    std::vector<std::string> names_icontains{"qRli"};
    query8.ifilter_name(libdnf::sack::QueryCmp::ICONTAINS, names_icontains);
    CPPUNIT_ASSERT_EQUAL(4lu, query8.size());
    auto pset7 = query8.get_package_set();
    for (auto pkg : pset7) {
        CPPUNIT_ASSERT(nevras_contains.find(pkg.get_nevra()) != nevras_contains.end());
    }
    query8.ifilter_name(libdnf::sack::QueryCmp::CONTAINS, names_icontains);
    CPPUNIT_ASSERT_EQUAL(0lu, query8.size());

    // Test unsupported cmp type
    CPPUNIT_ASSERT_THROW(query8.ifilter_name(libdnf::sack::QueryCmp::GT, names_icontains);
                         , libdnf::rpm::SolvQuery::NotSupportedCmpType);

    // Test QueryCmp::EQ with two elements
    libdnf::rpm::SolvQuery query9(sack.get());
    std::vector<std::string> names3{"CQRlib", "nodejs"};
    query9.ifilter_name(libdnf::sack::QueryCmp::EQ, names3);
    CPPUNIT_ASSERT_EQUAL(4lu, query9.size());
    auto pset8 = query9.get_package_set();
    for (auto pkg : pset8) {
        CPPUNIT_ASSERT(full_nevras.find(pkg.get_full_nevra()) != full_nevras.end());
    }
}

void RpmSolvQueryTest::test_ifilter_nevra() {
    std::set<std::string> nevras{"CQRlib-0:1.1.1-4.fc29.src", "CQRlib-0:1.1.1-4.fc29.x86_64"};

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> nevras_without_0_epoch{"CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64"};
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, nevras_without_0_epoch);
        CPPUNIT_ASSERT_EQUAL(2lu, query.size());
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT(nevras.find(pkg.get_full_nevra()) != nevras.end());
        }
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> nevras_with_0_epoch{"CQRlib-0:1.1.1-4.fc29.src", "CQRlib-0:1.1.1-4.fc29.x86_64"};
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, nevras_with_0_epoch);
        CPPUNIT_ASSERT_EQUAL(2lu, query.size());
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT(nevras.find(pkg.get_full_nevra()) != nevras.end());
        }
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - single argument
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> nevras_without_0_epoch{"CQRlib-1.1.1-4.fc29.src"};
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, nevras_without_0_epoch);
        CPPUNIT_ASSERT_EQUAL(1lu, query.size());
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT(pkg.get_full_nevra() == "CQRlib-0:1.1.1-4.fc29.src");
        }
    }

    {
        // Test QueryCmp::EQ - argument without 0 epoch - single argument
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> nevras_with_0_epoch{"CQRlib-0:1.1.1-4.fc29.src"};
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, nevras_with_0_epoch);
        CPPUNIT_ASSERT_EQUAL(1lu, query.size());
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT(pkg.get_full_nevra() == "CQRlib-0:1.1.1-4.fc29.src");
        }
    }

    {
        // Test QueryCmp::EQ - argument with unknown version - two elements
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> nevras_with_0_epoch{"CQRlib-0:1.1.1-unknown.src", "CQRlib-0:1.1.1-unknown1.x86_64"};
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, nevras_with_0_epoch);
        CPPUNIT_ASSERT_EQUAL(0lu, query.size());
    }

    {
        // Test QueryCmp::EQ - argument with unknown version - single argument
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> nevras_without_0_epoch{"CQRlib-1.1.1-unknown2.src"};
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, nevras_without_0_epoch);
        CPPUNIT_ASSERT_EQUAL(0lu, query.size());
    }

    {
        // Test QueryCmp::EQ - argument without epoch, but package with epoch - single argument
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> nevras_with_0_epoch{"nodejs-5.12.1-1.fc29.x86_64"};
        query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, nevras_with_0_epoch);
        CPPUNIT_ASSERT_EQUAL(0lu, query.size());
    }
}

void RpmSolvQueryTest::test_ifilter_version() {
    std::set<std::string> nevras{"CQRlib-0:1.1.1-4.fc29.src", "CQRlib-0:1.1.1-4.fc29.x86_64"};

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> version{"1.1.1"};
        query.ifilter_version(libdnf::sack::QueryCmp::EQ, version);
        CPPUNIT_ASSERT(query.size() == 2);
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT(nevras.find(pkg.get_full_nevra()) != nevras.end());
        }
    }
}

void RpmSolvQueryTest::test_ifilter_release() {
    std::set<std::string> nevras{"CQRlib-0:1.1.1-4.fc29.src", "CQRlib-0:1.1.1-4.fc29.x86_64", "lame-0:3.100-4.fc29.src", "lame-0:3.100-4.fc29.x86_64", "lame-libs-0:3.100-4.fc29.x86_64"};

    {
        // Test QueryCmp::EQ - argument without 0 epoch - two elements
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> release{"4.fc29"};
        query.ifilter_release(libdnf::sack::QueryCmp::EQ, release);
        CPPUNIT_ASSERT(query.size() == 5);
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT(nevras.find(pkg.get_full_nevra()) != nevras.end());
        }
    }
}

void RpmSolvQueryTest::test_ifilter_provides() {
    {
        // Test QueryCmp::EQ - string
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> provides{"glibc-langpack-hr"};
        query.ifilter_provides(libdnf::sack::QueryCmp::EQ, provides);
        CPPUNIT_ASSERT_EQUAL(1lu, query.size());
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT(pkg.get_full_nevra() == "glibc-langpack-hr-0:2.28-9.fc29.x86_64");
        }
    }

    {
        // Test QueryCmp::NEQ - string
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> provides{"glibc-langpack-hr"};
        query.ifilter_provides(libdnf::sack::QueryCmp::NEQ, provides);
        CPPUNIT_ASSERT_EQUAL(288lu, query.size());
    }
}

void RpmSolvQueryTest::test_ifilter_requires() {
    {
        // Test QueryCmp::EQ - string
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> requires {"wget"};
        query.ifilter_requires(libdnf::sack::QueryCmp::EQ, requires);
        CPPUNIT_ASSERT_EQUAL(1lu, query.size());
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT(pkg.get_full_nevra() == "abcde-0:2.9.2-1.fc29.noarch");
        }
    }

    {
        // Test QueryCmp::NEQ - string
        libdnf::rpm::SolvQuery query(sack.get());
        std::vector<std::string> requires {"wget"};
        query.ifilter_requires(libdnf::sack::QueryCmp::NEQ, requires);
        CPPUNIT_ASSERT_EQUAL(288lu, query.size());
    }
}

void RpmSolvQueryTest::test_resolve_pkg_spec() {
    {
        // Test NA
        libdnf::rpm::SolvQuery query(sack.get());
        auto return_value = query.resolve_pkg_spec("wget.x86_64", false, true, false, false, true, {});
        CPPUNIT_ASSERT_EQUAL(query.size(), 1lu);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT_EQUAL(pkg.get_full_nevra(), std::string("wget-0:1.19.5-5.fc29.x86_64"));
        }
    }

    {
        // Test a provide
        libdnf::rpm::SolvQuery query(sack.get());
        auto return_value = query.resolve_pkg_spec("wget > 1", false, true, true, false, true, {});
        CPPUNIT_ASSERT_EQUAL(query.size(), 1lu);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT_EQUAL(pkg.get_full_nevra(), std::string("wget-0:1.19.5-5.fc29.x86_64"));
        }
    }
    {
        // Test NEVRA glob
        libdnf::rpm::SolvQuery query(sack.get());
        auto return_value = query.resolve_pkg_spec("wge?-?:1.1?.5-?.fc29.x8?_64", false, true, false, false, true, {});
        CPPUNIT_ASSERT_EQUAL(query.size(), 1lu);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT_EQUAL(pkg.get_full_nevra(), std::string("wget-0:1.19.5-5.fc29.x86_64"));
        }
    }
    {
        // Test NEVRA glob - icase == false, nothing found
        libdnf::rpm::SolvQuery query(sack.get());
        auto return_value = query.resolve_pkg_spec("wGe?-?:1.1?.5-?.fc29.x8?_64", false, true, false, false, true, {});
        CPPUNIT_ASSERT_EQUAL(query.size(), 0lu);
    }
    {
        // Test NEVRA glob - icase == true
        libdnf::rpm::SolvQuery query(sack.get());
        auto return_value = query.resolve_pkg_spec("wGe?-?:1.1?.5-?.fc29.x8?_64", true, true, false, false, true, {});
        CPPUNIT_ASSERT_EQUAL(query.size(), 1lu);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT_EQUAL(pkg.get_full_nevra(), std::string("wget-0:1.19.5-5.fc29.x86_64"));
        }
    }
    {
        // Test NEVRA icase
        libdnf::rpm::SolvQuery query(sack.get());
        auto return_value = query.resolve_pkg_spec("wgeT-0:1.19.5-5.Fc29.X86_64", true, true, false, false, true, {});
        CPPUNIT_ASSERT_EQUAL(query.size(), 1lu);
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        auto pset = query.get_package_set();
        for (auto pkg : pset) {
            CPPUNIT_ASSERT_EQUAL(pkg.get_full_nevra(), std::string("wget-0:1.19.5-5.fc29.x86_64"));
        }
    }
}
