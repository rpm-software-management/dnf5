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
        std::string("Warning: skipped PGP checks for 1 package from repository: repomd-repo1"),
        transaction.get_gpg_signature_problems()[0]);
}

void BaseTransactionTest::test_check_gpg_signatures_fail() {
    add_repo_repomd("repomd-repo1");

    base.get_config().get_gpgcheck_option().set(true);

    libdnf5::Goal goal(base);
    goal.add_rpm_install("pkg");
    auto transaction = goal.resolve();

    CPPUNIT_ASSERT_EQUAL((size_t)1, transaction.get_transaction_packages_count());
    CPPUNIT_ASSERT(!transaction.check_gpg_signatures());
    CPPUNIT_ASSERT(!transaction.get_gpg_signature_problems().empty());
}
