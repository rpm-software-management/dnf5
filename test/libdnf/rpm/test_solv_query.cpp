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
    base = std::make_unique<libdnf::Base>();

    // Tunes main configuration. Sets path to cache directory.
    auto cwd = std::filesystem::current_path();
    base->get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, cwd.native());

    repo_sack = std::make_unique<libdnf::rpm::RepoSack>(*base);
    sack = std::make_unique<libdnf::rpm::SolvSack>(*base);

    // Creates new repository in the repo_sack
    auto repo = repo_sack->new_repo("dnf-ci-fedora");

    // Tunes repository configuration (baseurl is mandatory)
    auto repo_path = cwd / "../../../test/libdnf/rpm/repos-data/dnf-ci-fedora/";
    auto baseurl = "file://" + repo_path.native();
    auto repo_cfg = repo->get_config();
    repo_cfg->baseurl().set(libdnf::Option::Priority::RUNTIME, baseurl);

    // Loads repository into rpm::Repo.
    repo->load();

    // Loads rpm::Repo into rpm::SolvSack
    sack->load_repo(*repo.get(), false, libdnf::rpm::SolvSack::LoadRepoFlags::NONE);
}


void RpmSolvQueryTest::tearDown() {}


void RpmSolvQueryTest::test_size() {
    libdnf::rpm::SolvQuery query(sack.get());
    CPPUNIT_ASSERT(query.size() == 289);
}

void RpmSolvQueryTest::test_ifilter_name() {
    std::set<std::string> nevras{"CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64"};
    std::set<std::string> nevras_contains{
        "CQRlib-1.1.1-4.fc29.src",
        "CQRlib-1.1.1-4.fc29.x86_64",
        "CQRlib-devel-1.1.2-16.fc29.src",
        "CQRlib-devel-1.1.2-16.fc29.x86_64"
    };
    std::set<std::string> full_nevras{
        "CQRlib-0:1.1.1-4.fc29.src",
        "CQRlib-0:1.1.1-4.fc29.x86_64",
        "nodejs-1:5.12.1-1.fc29.src",
        "nodejs-1:5.12.1-1.fc29.x86_64"
    };

    /// Test QueryCmp::EQ
    libdnf::rpm::SolvQuery query(sack.get());
    std::vector<std::string> names{"CQRlib"};
    query.ifilter_name(libdnf::sack::QueryCmp::EQ, names);
    CPPUNIT_ASSERT(query.size() == 2);
    auto pset = query.get_package_set();
    for (auto pkg : pset) {
        CPPUNIT_ASSERT(nevras.find(pkg.get_nevra()) != nevras.end());
    }

    /// Test QueryCmp::GLOB
    libdnf::rpm::SolvQuery query2(sack.get());
    std::vector<std::string> names2{"CQ?lib"};
    query2.ifilter_name(libdnf::sack::QueryCmp::GLOB, names2);
    CPPUNIT_ASSERT(query2.size() == 2);
    auto pset2 = query2.get_package_set();
    for (auto pkg : pset2) {
        CPPUNIT_ASSERT(nevras.find(pkg.get_nevra()) != nevras.end());
    }

    /// Test two filters ifilter_name().ifilter_arch()
    std::vector<std::string> arches{"src"};
    libdnf::rpm::SolvQuery query3(sack.get());
    query3.ifilter_name(libdnf::sack::QueryCmp::EQ, names).ifilter_arch(libdnf::sack::QueryCmp::EQ, arches);
    CPPUNIT_ASSERT(query3.size() == 1);
    auto pset3 = query3.get_package_set();
    for (auto pkg : pset3) {
        CPPUNIT_ASSERT(pkg.get_nevra() == "CQRlib-1.1.1-4.fc29.src");
    }

    /// Test QueryCmp::NEQ
    libdnf::rpm::SolvQuery query4(sack.get());
    query4.ifilter_name(libdnf::sack::QueryCmp::NEQ, names);
    CPPUNIT_ASSERT(query4.size() == 287);

    /// Test QueryCmp::IEXACT
    libdnf::rpm::SolvQuery query5(sack.get());
    std::vector<std::string> names_icase{"cqrlib"};
    query5.ifilter_name(libdnf::sack::QueryCmp::IEXACT, names_icase);
    CPPUNIT_ASSERT(query5.size() == 2);
    auto pset4 = query5.get_package_set();
    for (auto pkg : pset4) {
        CPPUNIT_ASSERT(nevras.find(pkg.get_nevra()) != nevras.end());
    }
    query5.ifilter_name(libdnf::sack::QueryCmp::EQ, names_icase);
    CPPUNIT_ASSERT(query5.size() == 0);

    /// Test QueryCmp::IGLOB
    libdnf::rpm::SolvQuery query6(sack.get());
    std::vector<std::string> names_glob_icase{"cq?lib"};
    query6.ifilter_name(libdnf::sack::QueryCmp::IGLOB, names_glob_icase);
    CPPUNIT_ASSERT(query6.size() == 2);
    auto pset5 = query6.get_package_set();
    for (auto pkg : pset5) {
        CPPUNIT_ASSERT(nevras.find(pkg.get_nevra()) != nevras.end());
    }
    query6.ifilter_name(libdnf::sack::QueryCmp::GLOB, names_glob_icase);
    CPPUNIT_ASSERT(query6.size() == 0);

    /// Test QueryCmp::CONTAINS
    libdnf::rpm::SolvQuery query7(sack.get());
    std::vector<std::string> names_contains{"QRli"};
    query7.ifilter_name(libdnf::sack::QueryCmp::CONTAINS, names_contains);
    CPPUNIT_ASSERT(query7.size() == 4);
    auto pset6 = query7.get_package_set();
    for (auto pkg : pset6) {
        CPPUNIT_ASSERT(nevras_contains.find(pkg.get_nevra()) != nevras_contains.end());
    }

    /// Test QueryCmp::ICONTAINS
    libdnf::rpm::SolvQuery query8(sack.get());
    std::vector<std::string> names_icontains{"qRli"};
    query8.ifilter_name(libdnf::sack::QueryCmp::ICONTAINS, names_icontains);
    CPPUNIT_ASSERT(query8.size() == 4);
    auto pset7 = query8.get_package_set();
    for (auto pkg : pset7) {
        CPPUNIT_ASSERT(nevras_contains.find(pkg.get_nevra()) != nevras_contains.end());
    }
    query8.ifilter_name(libdnf::sack::QueryCmp::CONTAINS, names_icontains);
    CPPUNIT_ASSERT(query8.size() == 0);

    /// Test unsupported cmp type
    CPPUNIT_ASSERT_THROW(query8.ifilter_name(libdnf::sack::QueryCmp::GT, names_icontains);,
                         libdnf::rpm::SolvQuery::NotSupportedCmpType);

    /// Test QueryCmp::EQ with two elements
    libdnf::rpm::SolvQuery query9(sack.get());
    std::vector<std::string> names3{"CQRlib", "nodejs"};
    query9.ifilter_name(libdnf::sack::QueryCmp::EQ, names3);
    CPPUNIT_ASSERT(query9.size() == 4);
    auto pset8 = query9.get_package_set();
    for (auto pkg : pset8) {
        CPPUNIT_ASSERT(full_nevras.find(pkg.get_full_nevra()) != full_nevras.end());
    }

}
