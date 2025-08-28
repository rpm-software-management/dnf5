# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later


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

    def test_group_when_query_out_of_scope(self):
        self.add_repo_repomd("repomd-comps-core")

        def get_group(base):
            query = libdnf5.comps.GroupQuery(base)
            return next(iter(query))

        self.assertEqual(5, len(get_group(self.base).get_packages()))

    def test_iterating_over_inline_query(self):
        self.add_repo_repomd("repomd-comps-core", False)
        self.add_repo_repomd("repomd-comps-standard")

        ids = []
        for grp in sorted(libdnf5.comps.GroupQuery(self.base), key=lambda x: int(x.get_order_int())):
            ids.append(grp.get_groupid())

        self.assertEqual(['core', 'standard'], ids)
