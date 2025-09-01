# Copyright Contributors to the DNF5 project.
# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later
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

import unittest

import libdnf5.base

import base_test_case


class TestGoal(base_test_case.BaseTestCase):
    def test_missing_setup_goal_resolve(self):
        # Create a new Base object
        base = libdnf5.base.Base()

        # Create a new empty Goal
        goal = libdnf5.base.Goal(base)

        # Try to resolve the goal without running base.setup()
        with self.assertRaisesRegex(libdnf5.exception.UserAssertionError,
                                    'libdnf5/base/goal.cpp:[0-9]+:.*libdnf5::Goal::resolve\\(\\):'
                                    ' API Assertion \'p_impl->base->is_initialized\\(\\)\' failed:'
                                    ' Base instance was not fully initialized by Base::setup\\(\\)'):
            goal.resolve()

    def test_missing_rpm_goal_resolve(self):
        # Try to resolve the goal using unknown RPM package

        goal = libdnf5.base.Goal(self.base)
        goal.add_install('unknown_pkg.rpm')

        with self.assertRaisesRegex(libdnf5.exception.RepoRpmError,
                                    'Failed to access RPM "unknown_pkg.rpm":'
                                    ' No such file or directory'):
            goal.resolve()

    def test_invalid_url_goal_resolve(self):
        # Try to resolve the goal using invalid URL

        goal = libdnf5.base.Goal(self.base)
        goal.add_install('https://i-dont-exist.com')

        with self.assertRaisesRegex(libdnf5.exception.RepoFileDownloadErrorNested, 'Failed to download files') as cm:
            goal.resolve()
        self.assertRegex(cm.exception.format(libdnf5.exception.FormatDetailLevel_WithDomainAndName),
                         'libdnf5::repo::FileDownloadError: Failed to download files\n'
                         ' libdnf5::repo::LibrepoError: Librepo error: Curl error \\([0-9]+\\):'
                         ' .* https://i-dont-exist.com/')
        with self.assertRaisesRegex(libdnf5.exception.Error, 'Librepo error: Curl error') as cm_nested:
            cm.exception.rethrow_if_nested()
        self.assertRegex(cm_nested.exception.format(libdnf5.exception.FormatDetailLevel_WithDomainAndName),
                         'libdnf5::Error: Librepo error: Curl error \\([0-9]+\\):'
                         ' .* https://i-dont-exist.com/')

    def test_unsupported_argument_add_remove(self):
        # Try passing unsupported argument to add_remove() method

        goal = libdnf5.base.Goal(self.base)

        with self.assertRaisesRegex(libdnf5.exception.RuntimeError, 'Unsupported argument for REMOVE action: pkg.rpm'):
            goal.add_remove('pkg.rpm')

    def test_group_rpm_reason_change_without_id(self):
        # Try changing group reason without group_id

        goal = libdnf5.base.Goal(self.base)

        with self.assertRaisesRegex(libdnf5.exception.UserAssertionError,
                                    'libdnf5/base/goal.cpp:[0-9]+: void libdnf5::Goal::add_rpm_reason_change\\(.*\\):'
                                    ' API Assertion \'reason != libdnf5::transaction::TransactionItemReason::GROUP || !group_id.empty\\(\\)\''
                                    ' failed: group_id is required for setting reason "GROUP"'):
            goal.add_rpm_reason_change(
                '@fake-group-spec', libdnf5.base.transaction.TransactionItemReason_GROUP, '')

    def test_log_events_wrapper(self):
        # Try accessing transaction.get_resolve_logs()
        spec = 'unknown_spec'

        goal = libdnf5.base.Goal(self.base)
        goal.add_install(spec)

        transaction = goal.resolve()
        logs = transaction.get_resolve_logs()
        self.assertTrue(logs)
        first_log = next(iter(logs))
        self.assertEqual(libdnf5.base.GoalProblem_NOT_FOUND,
                         first_log.get_problem())
        self.assertEqual(spec, first_log.get_spec())
