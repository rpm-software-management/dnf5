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

import unittest

import libdnf5


# Test using attributes instead of getters/setters
class TestStructAttributes(unittest.TestCase):
    def test_resolvespecsettings(self):
        settings = libdnf5.base.ResolveSpecSettings()
        settings.ignore_case = True
        settings.with_provides = False
        settings.with_filenames = False
        self.assertEqual(True, settings.ignore_case)
        self.assertEqual(False, settings.with_provides)
        self.assertEqual(False, settings.with_filenames)

    def test_goaljobsettings(self):
        settings = libdnf5.base.GoalJobSettings()
        settings.best = libdnf5.base.GoalSetting_SET_TRUE
        settings.skip_unavailable = False
        settings.to_repo_ids = ('repo1', 'repo2')
        self.assertEqual(libdnf5.base.GoalSetting_SET_TRUE, settings.best)
        self.assertEqual(False, settings.skip_unavailable)
        self.assertEqual(('repo1', 'repo2'), settings.to_repo_ids)

    def test_rpmchangelog(self):
        changelog = libdnf5.rpm.Changelog(1234, 'Jo Anne', 'Version bump')
        self.assertEqual(1234, changelog.timestamp)
        self.assertEqual('Jo Anne', changelog.author)
        self.assertEqual('Version bump', changelog.text)
