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

void BaseTransactionTest::test_auto_load_filelists_on_file_dependency() {
    base.get_config().get_optional_metadata_types_option().set(std::set<std::string>{});
    add_repo_rpm("rpm-repo-filedep-provider", false);
    add_repo_rpm("rpm-repo-filedep-consumer");

    libdnf5::Goal goal(base);
    goal.add_rpm_install("consumer");
    auto transaction = goal.resolve();

    CPPUNIT_ASSERT_EQUAL((size_t)2, transaction.get_transaction_packages_count());
    CPPUNIT_ASSERT(transaction.get_filelists_auto_loaded());
}

void BaseTransactionTest::test_auto_load_filelists_not_needed_when_already_loaded() {
    // Filelists already loaded (the BaseTestCase default) - the dependency resolves on the
    // first try, no automatic retry should be needed.
    add_repo_rpm("rpm-repo-filedep-provider", false);
    add_repo_rpm("rpm-repo-filedep-consumer");

    libdnf5::Goal goal(base);
    goal.add_rpm_install("consumer");
    auto transaction = goal.resolve();

    CPPUNIT_ASSERT_EQUAL((size_t)2, transaction.get_transaction_packages_count());
    CPPUNIT_ASSERT(!transaction.get_filelists_auto_loaded());
}

void BaseTransactionTest::test_auto_load_filelists_gives_up_when_still_unresolved() {
    base.get_config().get_optional_metadata_types_option().set(std::set<std::string>{});
    add_repo_rpm("rpm-repo-filedep-provider", false);
    add_repo_rpm("rpm-repo-filedep-consumer");

    // "consumer-missing" requires a path no package provides, so loading filelists doesn't
    // help - resolution should still fail with a normal solver error, with no repeated
    // download attempts.
    libdnf5::Goal goal(base);
    goal.add_rpm_install("consumer-missing");
    auto transaction = goal.resolve();

    CPPUNIT_ASSERT(transaction.get_filelists_auto_loaded());
    CPPUNIT_ASSERT(
        (transaction.get_problems() & libdnf5::GoalProblem::SOLVER_ERROR) == libdnf5::GoalProblem::SOLVER_ERROR);
}

void BaseTransactionTest::test_auto_load_filelists_on_no_best_skip() {
    // With best=false, a file-based dependency that filelists could resolve doesn't cause a
    // hard failure - the solver just silently settles for a lesser candidate. This is only
    // detected by the strict-mode shadow-resolve in Transaction::Impl::set_transaction()
    // (see SOLVER_PROBLEM_STRICT_RESOLVEMENT), not by the normal solve's problems. Verify the
    // automatic retry is triggered by that case too, and that it recovers the actually-best
    // candidate rather than leaving the silently-downgraded transaction in place.
    base.get_config().get_optional_metadata_types_option().set(std::set<std::string>{});
    base.get_config().get_best_option().set(false);
    add_repo_rpm("rpm-repo-filedep-provider", false);
    add_repo_rpm("rpm-repo-filedep-consumer");

    libdnf5::Goal goal(base);
    goal.add_rpm_install("consumer-nobest");
    auto transaction = goal.resolve();

    CPPUNIT_ASSERT(transaction.get_filelists_auto_loaded());
    CPPUNIT_ASSERT_EQUAL((size_t)2, transaction.get_transaction_packages_count());
    bool found_v2 = false;
    for (const auto & tspkg : transaction.get_transaction_packages()) {
        if (tspkg.get_package().get_name() == "consumer-nobest") {
            CPPUNIT_ASSERT_EQUAL(std::string("2"), tspkg.get_package().get_version());
            found_v2 = true;
        }
    }
    CPPUNIT_ASSERT(found_v2);
}

void BaseTransactionTest::test_auto_load_filelists_disabled_by_config() {
    base.get_config().get_optional_metadata_types_option().set(std::set<std::string>{});
    base.get_config().get_filelists_auto_load_option().set(false);
    add_repo_rpm("rpm-repo-filedep-provider", false);
    add_repo_rpm("rpm-repo-filedep-consumer");

    // The file dependency would be resolvable via an automatic retry, but filelists_auto_load
    // is disabled, so resolution should fail with the original solver error and no retry.
    libdnf5::Goal goal(base);
    goal.add_rpm_install("consumer");
    auto transaction = goal.resolve();

    CPPUNIT_ASSERT(!transaction.get_filelists_auto_loaded());
    CPPUNIT_ASSERT(
        (transaction.get_problems() & libdnf5::GoalProblem::SOLVER_ERROR) == libdnf5::GoalProblem::SOLVER_ERROR);
}
