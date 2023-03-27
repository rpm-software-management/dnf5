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


import libdnf5

import base_test_case


class TestGroup(base_test_case.BaseTestCase):
    def test_group(self):
        self.add_repo_repomd("repomd-comps-core")
        q_core = libdnf5.comps.GroupQuery(self.base)
        _ = q_core.get()

    def test_group_query_without_setup(self):
        # Create a new Base object
        base = libdnf5.base.Base()

        # Try to create a group query without running base.setup()
        self.assertRaises(RuntimeError, libdnf5.comps.GroupQuery, base)

    def test_group_install_remove(self):
        # Check group is correctly registered after install
        # and so it could be removed after that
        self.add_repo_repomd("repomd-comps-core")

        goal_install = libdnf5.base.Goal(self.base)
        goal_install.add_install("@core")

        transaction_install = goal_install.resolve()
        self.assertEqual(transaction_install.get_problems(),
                         libdnf5.base.GoalProblem_NO_PROBLEM)
        transaction_install.run()
        self.assertFalse(transaction_install.get_transaction_problems())

        self.repo_sack.update_and_load_enabled_repos(True)

        goal_remove = libdnf5.base.Goal(self.base)
        goal_remove.add_remove("@core")

        transaction_remove = goal_remove.resolve()
        self.assertEqual(transaction_remove.get_problems(),
                         libdnf5.base.GoalProblem_NO_PROBLEM)
        transaction_remove.run()
        self.assertFalse(transaction_remove.get_transaction_problems())
