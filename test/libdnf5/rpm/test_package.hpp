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


#ifndef TEST_LIBDNF5_RPM_PACKAGE_HPP
#define TEST_LIBDNF5_RPM_PACKAGE_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class RpmPackageTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RpmPackageTest);
    CPPUNIT_TEST(test_equality);
    CPPUNIT_TEST(test_get_id);
    CPPUNIT_TEST(test_get_name);
    CPPUNIT_TEST(test_get_epoch);
    CPPUNIT_TEST(test_get_version);
    CPPUNIT_TEST(test_get_release);
    CPPUNIT_TEST(test_get_arch);
    CPPUNIT_TEST(test_get_evr);
    CPPUNIT_TEST(test_get_nevra);
    CPPUNIT_TEST(test_get_full_nevra);
    CPPUNIT_TEST(test_get_group);
    CPPUNIT_TEST(test_get_download_size);
    CPPUNIT_TEST(test_get_install_size);
    CPPUNIT_TEST(test_get_license);
    CPPUNIT_TEST(test_get_sourcerpm);
    CPPUNIT_TEST(test_get_build_time);
    //CPPUNIT_TEST(test_get_build_host);
    CPPUNIT_TEST(test_get_packager);
    CPPUNIT_TEST(test_get_vendor);
    CPPUNIT_TEST(test_get_url);
    CPPUNIT_TEST(test_get_summary);
    CPPUNIT_TEST(test_get_description);
    CPPUNIT_TEST(test_get_files);
    CPPUNIT_TEST(test_get_provides);
    CPPUNIT_TEST(test_get_requires);
    CPPUNIT_TEST(test_get_requires_pre);
    CPPUNIT_TEST(test_get_conflicts);
    CPPUNIT_TEST(test_get_obsoletes);
    CPPUNIT_TEST(test_get_prereq_ignoreinst);
    CPPUNIT_TEST(test_get_regular_requires);
    CPPUNIT_TEST(test_get_recommends);
    CPPUNIT_TEST(test_get_suggests);
    CPPUNIT_TEST(test_get_enhances);
    CPPUNIT_TEST(test_get_supplements);
    CPPUNIT_TEST(test_get_baseurl);
    CPPUNIT_TEST(test_get_location);
    CPPUNIT_TEST(test_get_checksum);
    CPPUNIT_TEST(test_get_hdr_checksum);
    CPPUNIT_TEST(test_is_installed);
    CPPUNIT_TEST(test_get_hdr_end);
    CPPUNIT_TEST(test_get_install_time);
    CPPUNIT_TEST(test_get_media_number);
    CPPUNIT_TEST(test_get_rpmdbid);

    CPPUNIT_TEST(test_to_nevra_string);
    CPPUNIT_TEST(test_to_full_nevra_string);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_equality();
    void test_get_id();
    void test_get_name();
    void test_get_epoch();
    void test_get_version();
    void test_get_release();
    void test_get_arch();
    void test_get_evr();
    void test_get_nevra();
    void test_get_full_nevra();
    void test_get_group();
    void test_get_download_size();
    void test_get_install_size();
    void test_get_license();
    void test_get_sourcerpm();
    void test_get_build_time();
    //void test_get_build_host();
    void test_get_packager();
    void test_get_vendor();
    void test_get_url();
    void test_get_summary();
    void test_get_description();
    void test_get_files();
    void test_get_provides();
    void test_get_requires();
    void test_get_requires_pre();
    void test_get_conflicts();
    void test_get_obsoletes();
    void test_get_prereq_ignoreinst();
    void test_get_regular_requires();
    void test_get_recommends();
    void test_get_suggests();
    void test_get_enhances();
    void test_get_supplements();
    void test_get_baseurl();
    void test_get_location();
    void test_get_checksum();
    void test_get_hdr_checksum();
    void test_is_installed();
    void test_get_hdr_end();
    void test_get_install_time();
    void test_get_media_number();
    void test_get_rpmdbid();

    void test_to_nevra_string();
    void test_to_full_nevra_string();
};


#endif  // TEST_LIBDNF5_RPM_PACKAGE_HPP
