# Copyright Contributors to the libdnf project.
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

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

        self.assertTrue(transaction.check_gpg_signatures())
        self.assertEqual((), transaction.get_gpg_signature_problems())

    def test_check_gpg_signatures_fail(self):
        self.base.get_config().gpgcheck = True

        goal = libdnf5.base.Goal(self.base)
        goal.add_rpm_install("pkg")
        transaction = goal.resolve()

        self.assertFalse(transaction.check_gpg_signatures())
        self.assertTrue(len(transaction.get_gpg_signature_problems()) > 0)
