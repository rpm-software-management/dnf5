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
    RepoFixture::setUp();
}

void BaseGoalTest::test_install() {
    add_repo_repomd("repomd-repo1");

    libdnf::Goal goal(*base);
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

    libdnf::Goal goal(*base);
    base->get_config().strict().set(libdnf::Option::Priority::RUNTIME, false);
    base->get_config().best().set(libdnf::Option::Priority::RUNTIME, true);
    base->get_config().clean_requirements_on_remove().set(libdnf::Option::Priority::RUNTIME, true);
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
    // this test covers an old dnf bug described in the following steps:
    // * specify a commandline package to install
    // * the specified package has the same NEVRA as an available package in a repo
    // * the package query uses NEVRA instead of id and is resolved into the available rather than the specified package
    // * -> an unexpected package is installed

    // add a repo with package 'one-0:1-1.noarch'
    add_repo_rpm("rpm-repo1");

    // add 'one-0:1-1.noarch' package from the command-line
    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/repos-rpm/rpm-repo1/one-1-1.noarch.rpm";
    auto cmdline_pkg = sack->add_cmdline_package(rpm_path, false);

    // install the command-line package
    libdnf::Goal goal(*base);
    goal.add_rpm_install(cmdline_pkg);

    // resolve the goal and read results
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
    base->get_config().multilib_policy().set(libdnf::Option::Priority::RUNTIME, "all");

    // install the command-line package
    libdnf::Goal goal(*base);
    goal.add_rpm_install("multilib");

    // resolve the goal and read results
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
    add_repo_repomd("repomd-repo1");

    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";

    // add the package to the @System repo so it appears as installed
    sack->add_system_package(rpm_path, false, false);

    // also add it to the @Commandline repo to make it available for reinstall
    sack->add_cmdline_package(rpm_path, false);

    libdnf::Goal goal(*base);
    goal.add_rpm_reinstall("cmdline");

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("cmdline-0:1.2-3.noarch"),
            TransactionItemAction::REINSTALL,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("cmdline-0:1.2-3.noarch", true),
            TransactionItemAction::REINSTALLED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_remove() {
    add_repo_repomd("repomd-repo1");

    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    sack->add_system_package(rpm_path, false, false);
    libdnf::Goal goal(*base);
    goal.add_rpm_remove("cmdline");
    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("cmdline-0:1.2-3.noarch", true),
            TransactionItemAction::REMOVE,
            TransactionItemReason::USER,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_remove_not_installed() {
    add_repo_repomd("repomd-repo1");

    base->get_config().clean_requirements_on_remove().set(libdnf::Option::Priority::RUNTIME, true);
    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";
    sack->add_system_package(rpm_path, false, false);
    libdnf::Goal goal(*base);
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
    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";

    // add the package to the @System repo so it appears installed
    sack->add_system_package(rpm_path, false, false);

    // also add it to the @Commandline repo to make it available for install
    sack->add_cmdline_package(rpm_path, false);

    libdnf::rpm::PackageQuery query(*base);
    query.filter_available().filter_nevra({"cmdline-0:1.2-3.noarch"});

    std::vector<std::string> expected = {"cmdline-0:1.2-3.noarch"};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector_string(query));

    libdnf::Goal goal(*base);
    goal.add_rpm_install(query);

    auto transaction = goal.resolve(false);

    CPPUNIT_ASSERT(transaction.get_packages().empty());
}

void BaseGoalTest::test_upgrade() {
    add_repo_repomd("repomd-repo1");

    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";

    // add the package to the @System repo so it appears installed
    sack->add_system_package(rpm_path, false, false);

    add_repo_solv("solv-upgrade");

    libdnf::rpm::PackageQuery query(*base);

    libdnf::Goal goal(*base);
    goal.add_rpm_upgrade("cmdline");

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("cmdline-0:1.2-4.noarch"),
            TransactionItemAction::UPGRADE,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("cmdline-0:1.2-3.noarch", true),
            TransactionItemAction::UPGRADED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_upgrade_not_available() {
    add_repo_repomd("repomd-repo1");

    base->get_config().best().set(libdnf::Option::Priority::RUNTIME, true);
    base->get_config().clean_requirements_on_remove().set(libdnf::Option::Priority::RUNTIME, true);
    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";

    // add the package to the @System repo so it appears installed
    sack->add_system_package(rpm_path, false, false);

    add_repo_solv("solv-upgrade");

    libdnf::rpm::PackageQuery query(*base);

    libdnf::Goal goal(*base);
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
    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";

    // add the package to the @System repo so it appears installed
    sack->add_system_package(rpm_path, false, false);

    add_repo_solv("solv-upgrade");

    libdnf::rpm::PackageQuery query(*base);

    libdnf::Goal goal(*base);
    goal.add_rpm_upgrade();

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("cmdline-0:1.2-4.noarch"),
            TransactionItemAction::UPGRADE,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("cmdline-0:1.2-3.noarch", true),
            TransactionItemAction::UPGRADED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_downgrade() {
    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";

    // add the package to the @System repo so it appears installed
    sack->add_system_package(rpm_path, false, false);

    add_repo_solv("solv-downgrade");

    libdnf::rpm::PackageQuery query(*base);

    libdnf::Goal goal(*base);
    goal.add_rpm_downgrade("cmdline");

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("cmdline-0:1.2-1.noarch"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("cmdline-0:1.2-3.noarch", true),
            TransactionItemAction::DOWNGRADED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_distrosync() {
    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";

    // add the package to the @System repo so it appears installed
    sack->add_system_package(rpm_path, false, false);

    add_repo_solv("solv-distrosync");

    libdnf::rpm::PackageQuery query(*base);

    libdnf::Goal goal(*base);
    goal.add_rpm_distro_sync("cmdline");

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("cmdline-0:1.2-1.noarch"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("cmdline-0:1.2-3.noarch", true),
            TransactionItemAction::DOWNGRADED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_distrosync_all() {
    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";

    // add the package to the @System repo so it appears installed
    sack->add_system_package(rpm_path, false, false);

    add_repo_solv("solv-distrosync");

    libdnf::rpm::PackageQuery query(*base);

    libdnf::Goal goal(*base);
    goal.add_rpm_distro_sync();

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("cmdline-0:1.2-1.noarch"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("cmdline-0:1.2-3.noarch", true),
            TransactionItemAction::DOWNGRADED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}

void BaseGoalTest::test_install_or_reinstall() {
    std::filesystem::path rpm_path = PROJECT_BINARY_DIR "/test/data/cmdline-rpms/cmdline-1.2-3.noarch.rpm";

    // add the package to the @System repo so it appears installed
    sack->add_system_package(rpm_path, false, false);

    // also add it to the @Commandline repo to make it available for reinstall
    sack->add_cmdline_package(rpm_path, false);

    libdnf::Goal goal(*base);
    libdnf::rpm::PackageQuery query(*base);
    query.filter_available().filter_nevra({"cmdline-0:1.2-3.noarch"});
    CPPUNIT_ASSERT_EQUAL(1lu, query.size());
    goal.add_rpm_install_or_reinstall(query);
    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {
        TransactionPackage(
            get_pkg("cmdline-0:1.2-3.noarch"),
            TransactionItemAction::REINSTALL,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        ),
        TransactionPackage(
            get_pkg("cmdline-0:1.2-3.noarch", true),
            TransactionItemAction::REINSTALLED,
            TransactionItemReason::UNKNOWN,
            TransactionItemState::UNKNOWN
        )
    };
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_packages());
}
