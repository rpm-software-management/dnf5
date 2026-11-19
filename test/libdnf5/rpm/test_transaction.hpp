// Copyright Contributors to the DNF5 project.
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


#ifndef LIBDNF5_TEST_RPM_TRANSACTION_HPP
#define LIBDNF5_TEST_RPM_TRANSACTION_HPP

#include "../shared/base_test_case.hpp"
#include "conf/config.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class RpmTransactionTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RpmTransactionTest);
    CPPUNIT_TEST(test_transaction);
    CPPUNIT_TEST(test_transaction_temp_files_cleanup);
    CPPUNIT_TEST(test_source_date_epoch_sorting);
    CPPUNIT_TEST(test_source_date_epoch_history_timestamps);
    CPPUNIT_TEST(test_pkgverify_level_enforced_when_gpgcheck_not_configured);
    CPPUNIT_TEST(test_pkgverify_level_enforced_for_cmdline_pkg_when_localpkg_gpgcheck_not_configured);
#if defined(HAVE_RPMTE_SETVFYLEVEL)
    CPPUNIT_TEST(test_pkgverify_level_not_enforced_when_repo_gpgcheck_explicitly_set_false);
    CPPUNIT_TEST(test_pkgverify_level_not_enforced_when_main_pkg_gpgcheck_explicitly_set_false);
    CPPUNIT_TEST(test_pkgverify_level_not_enforced_when_localpkg_gpgcheck_explicitly_set_false);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void test_transaction();
    void test_transaction_temp_files_cleanup();
    void test_source_date_epoch_sorting();
    void test_source_date_epoch_history_timestamps();
    void test_pkgverify_level_enforced_when_gpgcheck_not_configured();
    void test_pkgverify_level_enforced_for_cmdline_pkg_when_localpkg_gpgcheck_not_configured();
#if defined(HAVE_RPMTE_SETVFYLEVEL)
    void test_pkgverify_level_not_enforced_when_repo_gpgcheck_explicitly_set_false();
    void test_pkgverify_level_not_enforced_when_main_pkg_gpgcheck_explicitly_set_false();
    void test_pkgverify_level_not_enforced_when_localpkg_gpgcheck_explicitly_set_false();
#endif
};

#endif
