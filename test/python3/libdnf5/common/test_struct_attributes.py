# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

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
