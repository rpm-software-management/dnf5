# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

import unittest

import libdnf5.utils


class TestIsXPattern(unittest.TestCase):
    def test_is_file_pattern(self):
        self.assertFalse(libdnf5.utils.is_file_pattern(''))
        self.assertFalse(libdnf5.utils.is_file_pattern('no_file_pattern'))
        self.assertFalse(libdnf5.utils.is_file_pattern('no_file/pattern'))
        self.assertTrue(libdnf5.utils.is_file_pattern('/pattern'))
        self.assertTrue(libdnf5.utils.is_file_pattern('*/pattern'))

    def test_is_glob_pattern(self):
        self.assertFalse(libdnf5.utils.is_glob_pattern(''))
        self.assertFalse(libdnf5.utils.is_glob_pattern('no_glob_pattern'))
        self.assertTrue(libdnf5.utils.is_glob_pattern('glob*_pattern'))
        self.assertTrue(libdnf5.utils.is_glob_pattern('glob[sdf]_pattern'))
        self.assertTrue(libdnf5.utils.is_glob_pattern('glob?_pattern'))
