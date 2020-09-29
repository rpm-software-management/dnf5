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


#include "test_package.hpp"

#include "../utils.hpp"

#include "libdnf/rpm/solv_query.hpp"

#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(RpmPackageTest);


void RpmPackageTest::setUp() {
    RepoFixture::setUp();
    add_repo("dnf-ci-fedora");
    add_repo("package-test-baseurl");
}


libdnf::rpm::Package RpmPackageTest::get_pkg(const std::string &nevra) {
    libdnf::rpm::SolvQuery query(sack);
    query.ifilter_nevra(libdnf::sack::QueryCmp::EQ, {nevra});
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "get_pkg(\"" + nevra + "\"): no package or more than one package found.",
        1lu,
        query.size()
    );
    return *query.get_package_set().begin();
}


void RpmPackageTest::test_equality() {
    auto package1 = get_pkg("CQRlib-1.1.1-4.fc29.x86_64");
    auto package2 = get_pkg("CQRlib-1.1.1-4.fc29.x86_64");
    auto package3 = get_pkg("nodejs-1:5.12.1-1.fc29.x86_64");

    CPPUNIT_ASSERT(package1 == package2);
    CPPUNIT_ASSERT(!(package1 == package3));

    CPPUNIT_ASSERT(package1 != package3);
    CPPUNIT_ASSERT(!(package1 != package2));
}


void RpmPackageTest::test_get_id() {
    CPPUNIT_ASSERT(get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_id().id != 0);
}


void RpmPackageTest::test_get_name() {
    CPPUNIT_ASSERT_EQUAL(std::string("CQRlib"), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_name());
}


void RpmPackageTest::test_get_epoch() {
    CPPUNIT_ASSERT_EQUAL(0lu, get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_epoch());
    CPPUNIT_ASSERT_EQUAL(1lu, get_pkg("nodejs-1:5.12.1-1.fc29.x86_64").get_epoch());
}


void RpmPackageTest::test_get_version() {
    CPPUNIT_ASSERT_EQUAL(std::string("1.1.1"), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_version());
}


void RpmPackageTest::test_get_release() {
    CPPUNIT_ASSERT_EQUAL(std::string("4.fc29"), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_release());
}


void RpmPackageTest::test_get_arch() {
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_arch());
}


void RpmPackageTest::test_get_evr() {
    CPPUNIT_ASSERT_EQUAL(std::string("1.1.1-4.fc29"), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_evr());
    CPPUNIT_ASSERT_EQUAL(std::string("1:5.12.1-1.fc29"), get_pkg("nodejs-1:5.12.1-1.fc29.x86_64").get_evr());
}


void RpmPackageTest::test_get_nevra() {
    CPPUNIT_ASSERT_EQUAL(std::string("CQRlib-1.1.1-4.fc29.x86_64"), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_nevra());
}


void RpmPackageTest::test_get_full_nevra() {
    CPPUNIT_ASSERT_EQUAL(
        std::string("CQRlib-0:1.1.1-4.fc29.x86_64"),
        get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_full_nevra()
    );
}


void RpmPackageTest::test_get_group() {
    CPPUNIT_ASSERT_EQUAL(
        std::string("System Environment/Libraries"),
        get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_group()
    );
}


void RpmPackageTest::test_get_size() {
    CPPUNIT_ASSERT_EQUAL(6324llu, get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_size());
}


void RpmPackageTest::test_get_download_size() {
    CPPUNIT_ASSERT_EQUAL(6324llu, get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_download_size());
}


void RpmPackageTest::test_get_install_size() {
    // TODO implement installing packages for tests
    CPPUNIT_ASSERT_EQUAL(0llu, get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_install_size());
}


void RpmPackageTest::test_get_license() {
    CPPUNIT_ASSERT_EQUAL(std::string("LGPLv2+"), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_license());
}


void RpmPackageTest::test_get_sourcerpm() {
    CPPUNIT_ASSERT_EQUAL(
        std::string("CQRlib-1.1.1-4.fc29.src.rpm"),
        get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_sourcerpm()
    );
    CPPUNIT_ASSERT_EQUAL(std::string(), get_pkg("CQRlib-1.1.1-4.fc29.src").get_sourcerpm());
}


void RpmPackageTest::test_get_build_time() {
    CPPUNIT_ASSERT_EQUAL(1566991198llu, get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_build_time());
}


// TODO not supported by libsolv: https://github.com/openSUSE/libsolv/issues/400
//void RpmPackageTest::test_get_build_host() {
//    CPPUNIT_ASSERT_EQUAL(std::string(), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_build_host());
//}


void RpmPackageTest::test_get_packager() {
    CPPUNIT_ASSERT_EQUAL(std::string("Fedora Project"), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_packager());
}


void RpmPackageTest::test_get_vendor() {
    CPPUNIT_ASSERT_EQUAL(std::string("Fedora Project"), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_vendor());
}


void RpmPackageTest::test_get_url() {
    CPPUNIT_ASSERT_EQUAL(
        std::string("http://cqrlib.sourceforge.net/"),
        get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_url()
    );
}


void RpmPackageTest::test_get_summary() {
    CPPUNIT_ASSERT_EQUAL(
        std::string("ANSI C API for quaternion arithmetic and rotation"),
        get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_summary()
    );
}


void RpmPackageTest::test_get_description() {
    CPPUNIT_ASSERT_EQUAL(
        std::string("CQRlib is an ANSI C implementation of a utility library for quaternion\n"
            "arithmetic and quaternion rotation math."
        ),
        get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_description()
    );
}


void RpmPackageTest::test_get_files() {
    const auto files = get_pkg("glibc-2.28-9.fc29.x86_64").get_files();
    const std::vector<std::string> expected = {"/etc/ld.so.conf"};

    CPPUNIT_ASSERT_EQUAL(expected, files);
}


void RpmPackageTest::test_get_provides() {
    const auto provides = get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_provides();
    const std::vector<std::string> expected = {
        "libCQRlib.so.2()(64bit)",
        "CQRlib = 1.1.1-4.fc29",
        "CQRlib(x86-64) = 1.1.1-4.fc29",
    };

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(provides));
}


void RpmPackageTest::test_get_requires() {
    const auto reqs = get_pkg("requires-pkg-1.0-1.x86_64").get_requires();
    const std::vector<std::string> expected = {"test-requires", "test-prereq", "test-requires-pre"};

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(reqs));
}


void RpmPackageTest::test_get_requires_pre() {
    const auto prereqs = get_pkg("requires-pkg-1.0-1.x86_64").get_requires_pre();
    const std::vector<std::string> expected = {"test-prereq", "test-requires-pre"};

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(prereqs));
}


void RpmPackageTest::test_get_conflicts() {
    const auto conflicts = get_pkg("nodejs-1:5.12.1-1.fc29.x86_64").get_conflicts();
    const std::vector<std::string> expected = {"node <= 0.3.2-12"};

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(conflicts));
}


void RpmPackageTest::test_get_obsoletes() {
    const auto obsoletes = get_pkg("npm-1:5.12.1-1.fc29.x86_64").get_obsoletes();
    const std::vector<std::string> expected = {"npm < 3.5.4-6"};

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(obsoletes));
}


void RpmPackageTest::test_get_prereq_ignoreinst() {
    const auto prereqs = get_pkg("requires-pkg-1.0-1.x86_64").get_prereq_ignoreinst();
    // TODO requires the package to be installed
    //const std::vector<std::string> expected = {"test-requires-pre"};
    const std::vector<std::string> expected = {};

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(prereqs));
}


void RpmPackageTest::test_get_regular_requires() {
    const auto reg_reqs = get_pkg("requires-pkg-1.0-1.x86_64").get_regular_requires();
    const std::vector<std::string> expected = {"test-requires"};

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(reg_reqs));
}


void RpmPackageTest::test_get_recommends() {
    const auto recommends = get_pkg("abcde-2.9.2-1.fc29.noarch").get_recommends();
    const std::vector<std::string> expected = {"flac"};

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(recommends));
}


void RpmPackageTest::test_get_suggests() {
    const auto suggests = get_pkg("abcde-2.9.2-1.fc29.noarch").get_suggests();
    const std::vector<std::string> expected = {"lame"};

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(suggests));
}


void RpmPackageTest::test_get_enhances() {
    const auto enhances = get_pkg("enhances-pkg-1.0-1.x86_64").get_enhances();
    const std::vector<std::string> expected = {"test-enhances"};

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(enhances));
}


void RpmPackageTest::test_get_supplements() {
    const auto supplements = get_pkg("glibc-langpack-af-2.28-9.fc29.x86_64").get_supplements();
    const std::vector<std::string> expected = {"(glibc and (langpacks-af or langpacks-af_ZA))"};

    CPPUNIT_ASSERT_EQUAL(expected, to_vector(supplements));
}


void RpmPackageTest::test_get_baseurl() {
    CPPUNIT_ASSERT_EQUAL(std::string(), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_baseurl());
    CPPUNIT_ASSERT_EQUAL(std::string("http://dummy.com"), get_pkg("test-package-1:1.0-1.x86_64").get_baseurl());
}


void RpmPackageTest::test_get_location() {
    CPPUNIT_ASSERT_EQUAL(
        std::string("x86_64/CQRlib-1.1.1-4.fc29.x86_64.rpm"),
        get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_location()
    );
}


void RpmPackageTest::test_get_checksum() {
    CPPUNIT_ASSERT_EQUAL(
        std::string("8b1968310a8409bbca2c06d0217b010e83a82fbb515627d87c52ef7ba084e8b3"),
        get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_checksum().get_checksum()
    );
}


void RpmPackageTest::test_get_hdr_checksum() {
    // TODO header checksum is available only for installed packages
    CPPUNIT_ASSERT_EQUAL(std::string(""), get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_hdr_checksum().get_checksum());
}


void RpmPackageTest::test_is_installed() {
    // TODO implement installing packages for tests
    CPPUNIT_ASSERT_EQUAL(false, get_pkg("CQRlib-1.1.1-4.fc29.x86_64").is_installed());
}


void RpmPackageTest::test_get_hdr_end() {
    CPPUNIT_ASSERT_EQUAL(6208llu, get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_hdr_end());
}


void RpmPackageTest::test_get_install_time() {
    // TODO implement installing packages for tests
    CPPUNIT_ASSERT_EQUAL(0llu, get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_install_time());
}


void RpmPackageTest::test_get_media_number() {
    // TODO zeros everywhere
    CPPUNIT_ASSERT_EQUAL(0llu, get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_media_number());
}


void RpmPackageTest::test_get_rpmdbid() {
    // TODO zeros everywhere
    CPPUNIT_ASSERT_EQUAL(0llu, get_pkg("CQRlib-1.1.1-4.fc29.x86_64").get_rpmdbid());
}
