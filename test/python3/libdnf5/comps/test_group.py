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

import base_test_case


class TestGroup(base_test_case.BaseTestCase):
    def test_group(self):
        self.add_repo_repomd("repomd-comps-core")
        q_core = libdnf5.comps.GroupQuery(self.base)
        core = q_core.get()

    def test_group_query_without_setup(self):
        # Create a new Base object
        base = libdnf5.base.Base()

        # Try to create a group query without running base.setup()
        with self.assertRaisesRegex(libdnf5.exception.UserAssertionError,
                                    'base_impl.hpp:[0-9]+: libdnf5::solv::CompsPool& libdnf5::Base::Impl::get_comps_pool\\(\\):'
                                    ' API Assertion \'comps_pool\' failed:'
                                    ' Base instance was not fully initialized by Base::setup\\(\\)'):
            libdnf5.comps.GroupQuery(base)

    def test_group_get_packages(self):
        self.add_repo_repomd("repomd-comps-core")
        query = libdnf5.comps.GroupQuery(self.base)
        core_group = next(iter(query))
        self.assertEqual(5, len(core_group.get_packages()))
