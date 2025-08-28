# Copyright Contributors to the DNF5 project.
# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

import libdnf5.base

import base_test_case


class TestTransaction(base_test_case.BaseTestCase):
    def setUp(self):
        super().setUp()
        self.add_repo_repomd("repomd-repo1")

    def test_check_gpg_signatures_no_gpgcheck(self):
        goal = libdnf5.base.Goal(self.base)
        goal.add_rpm_install("pkg")
        transaction = goal.resolve()

        self.assertEqual(1, transaction.get_transaction_packages_count())
        self.assertTrue(transaction.check_gpg_signatures())
        self.assertEqual(('Warning: skipped OpenPGP checks for 1 package from repository: repomd-repo1',),
                         transaction.get_gpg_signature_problems())

    def test_check_gpg_signatures_fail(self):
        # Both options need to be available
        self.base.get_config().gpgcheck = True
        self.base.get_config().pkg_gpgcheck = True

        goal = libdnf5.base.Goal(self.base)
        goal.add_rpm_install("pkg")
        transaction = goal.resolve()

        self.assertEqual(1, transaction.get_transaction_packages_count())
        self.assertFalse(transaction.check_gpg_signatures())
        self.assertTrue(len(transaction.get_gpg_signature_problems()) > 0)
