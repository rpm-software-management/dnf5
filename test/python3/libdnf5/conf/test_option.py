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

import libdnf5

import base_test_case


class TestConfigurationOptions(base_test_case.BaseTestCase):
    def test_set_with_runtime_priority(self):
        proxy = self.base.get_config().get_proxy_option()
        self.assertEqual(proxy.get_priority(),
                         libdnf5.conf.Option.Priority_DEFAULT)

        proxy.set('abcd')
        self.assertEqual(proxy.get_value(), 'abcd')
        self.assertEqual(proxy.get_priority(),
                         libdnf5.conf.Option.Priority_RUNTIME)

    def test_container_add_item(self):
        auths_config = self.base.get_config().get_proxy_auth_method_option()
        auths_config.set(('basic', 'ntlm'))
        auths_config.add_item(libdnf5.conf.Option.Priority_RUNTIME, 'digest')
        self.assertEqual(auths_config.get_value(), ('basic', 'digest', 'ntlm'))

    def test_container_add(self):
        types_config = self.base.get_config().get_optional_metadata_types_option()
        # optional_metadata_types is an append option with non-empty default.
        # To test anything, clear it first
        types_config.set(())
        types_config.set((libdnf5.conf.METADATA_TYPE_FILELISTS,))
        types_config.add(libdnf5.conf.Option.Priority_RUNTIME, (libdnf5.conf.METADATA_TYPE_COMPS,
                         libdnf5.conf.METADATA_TYPE_UPDATEINFO))
        self.assertEqual(types_config.get_value(), (libdnf5.conf.METADATA_TYPE_COMPS,
                         libdnf5.conf.METADATA_TYPE_FILELISTS, libdnf5.conf.METADATA_TYPE_UPDATEINFO))

    def test_set_by_attribute(self):
        config = self.base.get_config()
        config.comment = 'some comment'
        comment_option = config.get_comment_option()
        self.assertEqual(comment_option.get_value(), 'some comment')
        self.assertEqual(comment_option.get_priority(),
                         libdnf5.conf.Option.Priority_RUNTIME)

    def test_get_by_attribute(self):
        config = self.base.get_config()
        config.get_comment_option().set('new comment')
        self.assertEqual(config.comment, 'new comment')

    def test_set_get_by_attribute(self):
        config = self.base.get_config()
        config.comment = 'test'
        self.assertEqual(config.comment, 'test')

    def test_writing_to_locked_option(self):
        config = self.base.get_config()

        option = config.get_keepcache_option()
        option.lock('')

        with self.assertRaisesRegex(libdnf5.exception.UserAssertionError,
                                    'libdnf5/conf/option.cpp:[0-9]+: void libdnf5::Option::assert_not_locked\\(\\) const:'
                                    ' API Assertion \'!p_impl->locked\' failed:'
                                    ' Attempting to write to a locked option'):
            option.set(False)

    def test_get_unset_option_by_attribute(self):
        config = self.base.get_config()
        destdir = config.destdir
        self.assertEqual(destdir, None)

    def test_get_unknown_option_by_attribute(self):
        config = self.base.get_config()
        self.assertRaises(AttributeError, lambda: config.xyz)

    def test_iterate_options(self):
        config = self.base.get_config()

        config.proxy = 'abcd'

        proxy_option = None
        for option in config.opt_binds():
            if option.first == 'proxy':
                proxy_option = (option.first, option.second.get_value_string())
                break

        self.assertEqual(proxy_option, ('proxy', 'abcd'))
