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


#include "test_goal.hpp"

#include "test/libdnf/utils.hpp"

#include "libdnf/base/goal.hpp"

#include <libdnf/rpm/package_query.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(BaseGoalTest);


namespace {

using namespace libdnf::transaction;

// make constructor public so we can create Package instances in the tests
class TransactionPackage : public libdnf::base::TransactionPackage {
public:
    TransactionPackage(const libdnf::rpm::Package & pkg, Action action, Reason reason, State state)
    : libdnf::base::TransactionPackage(pkg, action, reason) {
        this->state = state;
    }
};

}  // namespace


void BaseGoalTest::setUp() {
    LibdnfTestCase::setUp();
}

void BaseGoalTest::test_install() {
    add_repo_repomd("repomd-repo1");

    libdnf::Goal goal(base);
    goal.add_rpm_install("pkg");
    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("pkg-0:1.2-3.x86_64"),
            TransactionItemAction::INSTALL,
            TransactionItemReason::USER,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_install_not_available() {
    add_repo_repomd("repomd-repo1");

    libdnf::Goal goal(base);
    base.get_config().strict().set(libdnf::Option::Priority::RUNTIME, false);
    base.get_config().best().set(libdnf::Option::Priority::RUNTIME, true);
    base.get_config().clean_requirements_on_remove().set(libdnf::Option::Priority::RUNTIME, true);
    goal.add_rpm_install("not_available");
    auto transaction = goal.resolve(false);

    CPPUNIT_ASSERT(transaction.get_packages().empty());

    auto & log = goal.get_resolve_log();
    CPPUNIT_ASSERT_EQUAL(1lu, log.size());
    auto & [action, problem, settings, spec, additional_data] = *log.begin();
    CPPUNIT_ASSERT_EQUAL(libdnf::Goal::Action::INSTALL, action);
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalProblem::NOT_FOUND, problem);
    CPPUNIT_ASSERT_EQUAL(std::string("not_available"), spec);
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalUsedSetting::USED_FALSE, settings.get_used_clean_requirements_on_remove());
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalUsedSetting::USED_TRUE, settings.get_used_best());
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalUsedSetting::USED_FALSE, settings.get_used_strict());
}

void BaseGoalTest::test_install_from_cmdline() {
    // Tests installing a cmdline package when there is a package with the same NEVRA available in a repo
    add_repo_rpm("rpm-repo1");
    auto cmdline_pkg = add_cmdline_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf::Goal goal(base);
    goal.add_rpm_install(cmdline_pkg);

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("one-0:1-1.noarch", "@commandline"),
            TransactionItemAction::INSTALL,
            TransactionItemReason::USER,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_install_multilib_all() {
    add_repo_solv("solv-multilib");
    base.get_config().multilib_policy().set(libdnf::Option::Priority::RUNTIME, "all");

    libdnf::Goal goal(base);
    goal.add_rpm_install("multilib");

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("multilib-0:1.2-4.i686"),
            TransactionItemAction::INSTALL,
            TransactionItemReason::USER,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("multilib-0:1.2-4.x86_64"),
            TransactionItemAction::INSTALL,
            TransactionItemReason::USER,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_reinstall() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf::Goal goal(base);
    goal.add_rpm_reinstall("one");

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::REINSTALL,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REINSTALLED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_remove() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf::Goal goal(base);
    goal.add_rpm_remove("one");
    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REMOVE,
            TransactionItemReason::USER,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_remove_not_installed() {
    base.get_config().clean_requirements_on_remove().set(libdnf::Option::Priority::RUNTIME, true);

    libdnf::Goal goal(base);
    goal.add_rpm_remove("not_installed");
    auto transaction = goal.resolve(false);

    CPPUNIT_ASSERT(transaction.get_packages().empty());

    auto & log = goal.get_resolve_log();
    CPPUNIT_ASSERT_EQUAL(1lu, log.size());
    auto & [action, problem, settings, spec, additional_data] = *log.begin();
    CPPUNIT_ASSERT_EQUAL(libdnf::Goal::Action::REMOVE, action);
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalProblem::NOT_FOUND, problem);
    CPPUNIT_ASSERT_EQUAL(std::string("not_installed"), spec);
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalUsedSetting::USED_TRUE, settings.get_used_clean_requirements_on_remove());
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalUsedSetting::UNUSED, settings.get_used_best());
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalUsedSetting::UNUSED, settings.get_used_strict());
}

void BaseGoalTest::test_install_installed_pkg() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf::rpm::PackageQuery query(base);
    query.filter_available().filter_nevra({"one-0:1-1.noarch"});

    std::vector<std::string> expected = {"one-0:1-1.noarch"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));

    libdnf::Goal goal(base);
    goal.add_rpm_install(query);

    auto transaction = goal.resolve(false);

    CPPUNIT_ASSERT(transaction.get_packages().empty());
}

void BaseGoalTest::test_upgrade() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf::rpm::PackageQuery query(base);

    libdnf::Goal goal(base);
    goal.add_rpm_upgrade("one");

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("one-0:2-1.noarch"),
            TransactionItemAction::UPGRADE,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::UPGRADED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_upgrade_not_available() {
    base.get_config().best().set(libdnf::Option::Priority::RUNTIME, true);
    base.get_config().clean_requirements_on_remove().set(libdnf::Option::Priority::RUNTIME, true);

    libdnf::rpm::PackageQuery query(base);

    libdnf::Goal goal(base);
    goal.add_rpm_upgrade("not_available");

    auto transaction = goal.resolve(false);

    CPPUNIT_ASSERT(transaction.get_packages().empty());

    auto & log = goal.get_resolve_log();
    CPPUNIT_ASSERT_EQUAL(1lu, log.size());
    auto & [action, problem, settings, spec, additional_data] = *log.begin();
    CPPUNIT_ASSERT_EQUAL(libdnf::Goal::Action::UPGRADE, action);
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalProblem::NOT_FOUND, problem);
    CPPUNIT_ASSERT_EQUAL(std::string("not_available"), spec);
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalUsedSetting::USED_FALSE, settings.get_used_clean_requirements_on_remove());
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalUsedSetting::USED_TRUE, settings.get_used_best());
    CPPUNIT_ASSERT_EQUAL(libdnf::GoalUsedSetting::UNUSED, settings.get_used_strict());
}

void BaseGoalTest::test_upgrade_all() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf::rpm::PackageQuery query(base);

    libdnf::Goal goal(base);
    goal.add_rpm_upgrade();

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("one-0:2-1.noarch"),
            TransactionItemAction::UPGRADE,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::UPGRADED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_downgrade() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-2-1.noarch.rpm");

    libdnf::rpm::PackageQuery query(base);

    libdnf::Goal goal(base);
    goal.add_rpm_downgrade("one");

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("one-0:2-1.noarch", true),
            TransactionItemAction::DOWNGRADED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_distrosync() {
    add_repo_solv("solv-distrosync");
    add_system_pkg("repos-rpm/rpm-repo1/one-2-1.noarch.rpm");

    libdnf::rpm::PackageQuery query(base);

    libdnf::Goal goal(base);
    goal.add_rpm_distro_sync("one");

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("one-0:2-1.noarch", true),
            TransactionItemAction::DOWNGRADED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_distrosync_all() {
    add_repo_solv("solv-distrosync");
    add_system_pkg("repos-rpm/rpm-repo1/one-2-1.noarch.rpm");

    libdnf::rpm::PackageQuery query(base);

    libdnf::Goal goal(base);
    goal.add_rpm_distro_sync();

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("one-0:2-1.noarch", true),
            TransactionItemAction::DOWNGRADED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_install_or_reinstall() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf::Goal goal(base);
    libdnf::rpm::PackageQuery query(base);
    query.filter_available().filter_nevra({"one-0:1-1.noarch"});
    CPPUNIT_ASSERT_EQUAL(1lu, query.size());
    goal.add_rpm_install_or_reinstall(query);
    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::REINSTALL,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REINSTALLED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}
