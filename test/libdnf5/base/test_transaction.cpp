// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#include "test_transaction.hpp"

#include "../shared/utils.hpp"

#include <libdnf5/base/goal.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(BaseTransactionTest);

void BaseTransactionTest::test_check_gpg_signatures_no_gpgcheck() {
    add_repo_repomd("repomd-repo1");

    libdnf5::Goal goal(base);
    goal.add_rpm_install("pkg");
    auto transaction = goal.resolve();

    CPPUNIT_ASSERT_EQUAL((size_t)1, transaction.get_transaction_packages_count());
    CPPUNIT_ASSERT(transaction.check_gpg_signatures());
    CPPUNIT_ASSERT_EQUAL((size_t)1, transaction.get_gpg_signature_problems().size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("Warning: skipped OpenPGP checks for 1 package from repository: repomd-repo1"),
        transaction.get_gpg_signature_problems()[0]);
}

void BaseTransactionTest::test_check_gpg_signatures_fail() {
    add_repo_repomd("repomd-repo1");

    base.get_config().get_pkg_gpgcheck_option().set(true);

    libdnf5::Goal goal(base);
    goal.add_rpm_install("pkg");
    auto transaction = goal.resolve();

    CPPUNIT_ASSERT_EQUAL((size_t)1, transaction.get_transaction_packages_count());
    CPPUNIT_ASSERT(!transaction.check_gpg_signatures());
    CPPUNIT_ASSERT(!transaction.get_gpg_signature_problems().empty());
}
