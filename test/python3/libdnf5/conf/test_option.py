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


PRIORITY = libdnf5.conf.Option


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


class TestOption(base_test_case.BaseTestCase):
    def test_option_bool(self):
        option = libdnf5.conf.OptionBool(False)
        self.assertEqual(False, option.get_default_value())
        self.assertEqual(False, option.get_value())
        self.assertEqual(PRIORITY.Priority_DEFAULT, option.get_priority())
        self.assertEqual('', option.get_source())

        option.set(PRIORITY.Priority_COMMANDLINE, 'TrUe', 'cmdline')
        self.assertEqual(False, option.get_default_value())
        self.assertEqual(True, option.get_value())
        self.assertEqual('cmdline', option.get_source())

        # Lower priority does not change value or source
        option.set(PRIORITY.Priority_MAINCONFIG, False, 'mainconfig')
        self.assertEqual(True, option.get_value())
        self.assertEqual('cmdline', option.get_source())

        option.set(PRIORITY.Priority_RUNTIME, 'FalSe', 'runtime_src')
        self.assertEqual(False, option.get_value())
        self.assertEqual(PRIORITY.Priority_RUNTIME, option.get_priority())
        self.assertEqual('runtime_src', option.get_source())

        self.assertRaises(libdnf5.exception.OptionInvalidValueError,
                          option.set, PRIORITY.Priority_RUNTIME, 'invalid')

        option.lock('option locked by test_option_bool')
        self.assertRaises(libdnf5.exception.UserAssertionError,
                          option.set, PRIORITY.Priority_RUNTIME, True)
        self.assertRaises(libdnf5.exception.UserAssertionError,
                          option.set, True)

    def test_option_enum(self):
        option = libdnf5.conf.OptionEnum('bb', ['aa', 'bb', 'cc', 'dd', 'ee'])
        self.assertEqual('bb', option.get_default_value())
        self.assertEqual('bb', option.get_value())
        self.assertEqual(PRIORITY.Priority_DEFAULT, option.get_priority())
        self.assertEqual('', option.get_source())

        option.set(PRIORITY.Priority_RUNTIME, 'cc', 'enum_src')
        self.assertEqual('cc', option.get_value())
        self.assertEqual('enum_src', option.get_source())

        self.assertRaises(libdnf5.exception.OptionValueNotAllowedError,
                          option.set, PRIORITY.Priority_RUNTIME, 'not_allowed')
        # Source unchanged after failed set
        self.assertEqual('enum_src', option.get_source())

        option.set(PRIORITY.Priority_RUNTIME, 'aa')
        option.lock('option locked by test_option_enum')
        self.assertRaises(libdnf5.exception.UserAssertionError,
                          option.set, PRIORITY.Priority_RUNTIME, 'aa')

    def test_option_number(self):
        option = libdnf5.conf.OptionNumberInt32(10, -25, 50)
        self.assertEqual(10, option.get_default_value())
        self.assertEqual(10, option.get_value())
        self.assertEqual(PRIORITY.Priority_DEFAULT, option.get_priority())
        self.assertEqual('', option.get_source())

        option.set(PRIORITY.Priority_COMMANDLINE, 5, 'number_src')
        self.assertEqual(5, option.get_value())
        self.assertEqual('number_src', option.get_source())

        # Lower priority does not change value or source
        option.set(PRIORITY.Priority_MAINCONFIG, 3, 'lower_src')
        self.assertEqual(5, option.get_value())
        self.assertEqual('number_src', option.get_source())

        self.assertRaises(libdnf5.exception.OptionValueNotAllowedError,
                          option.set, PRIORITY.Priority_RUNTIME, -26)
        self.assertRaises(libdnf5.exception.OptionValueNotAllowedError,
                          option.set, PRIORITY.Priority_RUNTIME, 51)

        option.set(PRIORITY.Priority_RUNTIME, 1)
        option.lock('option locked by test_option_number')
        self.assertRaises(libdnf5.exception.UserAssertionError,
                          option.set, PRIORITY.Priority_RUNTIME, 1)

    def test_option_path(self):
        option = libdnf5.conf.OptionPath('/default_path', False, True)
        self.assertEqual('/default_path', option.get_default_value())
        self.assertEqual('/default_path', option.get_value())
        self.assertEqual(PRIORITY.Priority_DEFAULT, option.get_priority())
        self.assertEqual('', option.get_source())

        option.set(PRIORITY.Priority_RUNTIME, '/path2', 'path_src')
        self.assertEqual('/path2', option.get_value())
        self.assertEqual(PRIORITY.Priority_RUNTIME, option.get_priority())
        self.assertEqual('path_src', option.get_source())

        self.assertRaises(libdnf5.exception.OptionValueNotAllowedError,
                          option.set, PRIORITY.Priority_RUNTIME, 'not_absolute')
        # Source unchanged after failed set
        self.assertEqual('path_src', option.get_source())

        option.lock('option locked by test_option_path')
        self.assertRaises(libdnf5.exception.UserAssertionError,
                          option.set, PRIORITY.Priority_RUNTIME, '/path2')

    def test_option_string(self):
        option = libdnf5.conf.OptionString('default', 'd.*t', False)
        self.assertEqual('default', option.get_default_value())
        self.assertEqual('default', option.get_value())
        self.assertEqual(PRIORITY.Priority_DEFAULT, option.get_priority())
        self.assertEqual('', option.get_source())

        option.set(PRIORITY.Priority_COMMANDLINE, 'donut', 'string_src')
        self.assertEqual('default', option.get_default_value())
        self.assertEqual('donut', option.get_value())
        self.assertEqual('string_src', option.get_source())

        # Case sensitive - uppercase 'T' is not allowed
        self.assertRaises(libdnf5.exception.OptionValueNotAllowedError,
                          option.set, PRIORITY.Priority_RUNTIME, 'donuT')
        # Source unchanged after failed set
        self.assertEqual('string_src', option.get_source())

        option.lock('option locked by test_option_string')
        self.assertRaises(libdnf5.exception.UserAssertionError,
                          option.set, PRIORITY.Priority_RUNTIME, 'doXXnut')

    def test_option_string_list(self):
        option = libdnf5.conf.OptionStringList(('dval1X', 'dval2X'), '[d|c].*X', False)
        self.assertEqual(('dval1X', 'dval2X'), option.get_default_value())
        self.assertEqual(('dval1X', 'dval2X'), option.get_value())
        self.assertEqual(PRIORITY.Priority_DEFAULT, option.get_priority())
        self.assertEqual('', option.get_source())

        option.set(PRIORITY.Priority_RUNTIME, ('donutX', 'cakeX'), 'list_src')
        self.assertEqual(('dval1X', 'dval2X'), option.get_default_value())
        self.assertEqual(('donutX', 'cakeX'), option.get_value())
        self.assertEqual(PRIORITY.Priority_RUNTIME, option.get_priority())
        self.assertEqual('list_src', option.get_source())

        # Case sensitive - lowercase 'x' is not allowed
        self.assertRaises(libdnf5.exception.OptionValueNotAllowedError,
                          option.set, PRIORITY.Priority_RUNTIME, ('donutX', 'cakex'))
        # Source unchanged after failed set
        self.assertEqual('list_src', option.get_source())

        option.set(PRIORITY.Priority_RUNTIME, 'dfirstX, dsecondX', 'string_list_src')
        self.assertEqual(('dfirstX', 'dsecondX'), option.get_value())
        self.assertEqual('string_list_src', option.get_source())

        option.lock('option locked by test_option_string_list')
        self.assertRaises(libdnf5.exception.UserAssertionError,
                          option.set, PRIORITY.Priority_RUNTIME, 'doXXnut')

    def test_option_string_set(self):
        option = libdnf5.conf.OptionStringSet(('x', 'y', 'z'))
        self.assertEqual('', option.get_source())

        option.set(PRIORITY.Priority_RUNTIME, 'a, b, c', 'set_src')
        self.assertEqual('set_src', option.get_source())

    def test_option_list_add(self):
        option = libdnf5.conf.OptionStringSet('1, 2, 3')

        option.add(PRIORITY.Priority_RUNTIME, ('4', '5', '6'), 'add_src')
        self.assertEqual(('1', '2', '3', '4', '5', '6'), option.get_value())
        self.assertEqual('add_src', option.get_source())

    def test_option_string_append_list(self):
        option = libdnf5.conf.OptionStringAppendList('Pkg1, Pkg2')
        self.assertEqual(('Pkg1', 'Pkg2'), option.get_value())
        self.assertEqual('', option.get_source())

        # setting a new value will append to current value
        option.set(PRIORITY.Priority_COMMANDLINE, 'Pkg3', 'cmdline_src')
        self.assertEqual(('Pkg1', 'Pkg2', 'Pkg3'), option.get_value())
        self.assertEqual('cmdline_src', option.get_source())

        # values are evaluated ordered by the priority
        option.set(PRIORITY.Priority_MAINCONFIG, 'Pkg4', 'mainconfig_src')
        self.assertEqual(('Pkg1', 'Pkg2', 'Pkg4', 'Pkg3'), option.get_value())
        # COMMANDLINE has higher priority, source stays from COMMANDLINE
        self.assertEqual('cmdline_src', option.get_source())

        # I can clear the option an using empty value
        option.set(PRIORITY.Priority_COMMANDLINE, '', 'clear_src')
        self.assertEqual((), option.get_value())
        self.assertEqual('clear_src', option.get_source())

    def test_option_child_bool(self):
        parent = libdnf5.conf.OptionBool(True)
        child = libdnf5.conf.OptionChildBool(parent)
        self.assertEqual(True, child.get_default_value())
        self.assertEqual(True, child.get_value())
        self.assertEqual(PRIORITY.Priority_DEFAULT, child.get_priority())
        self.assertEqual('', child.get_source())
        self.assertEqual(False, child.has_own_value())

        parent.set(PRIORITY.Priority_RUNTIME, False, 'parent_src')
        self.assertEqual(True, child.get_default_value())
        self.assertEqual(False, child.get_value())
        self.assertEqual(PRIORITY.Priority_RUNTIME, child.get_priority())
        self.assertEqual('parent_src', child.get_source())
        self.assertEqual(False, child.has_own_value())

        child.set(PRIORITY.Priority_COMMANDLINE, False, 'child_src')
        self.assertEqual(True, child.get_default_value())
        self.assertEqual(False, child.get_value())
        self.assertEqual(PRIORITY.Priority_COMMANDLINE, child.get_priority())
        self.assertEqual('child_src', child.get_source())
        self.assertEqual(True, child.has_own_value())

        # Lower priority does not change value or source
        child.set(PRIORITY.Priority_MAINCONFIG, True, 'lower_src')
        self.assertEqual(False, child.get_value())
        self.assertEqual(PRIORITY.Priority_COMMANDLINE, child.get_priority())
        self.assertEqual('child_src', child.get_source())

        child.set(PRIORITY.Priority_COMMANDLINE, True)
        self.assertEqual(True, child.get_value())

        self.assertRaises(libdnf5.exception.OptionInvalidValueError,
                          child.set, PRIORITY.Priority_COMMANDLINE, 'invalid')

        child.set(PRIORITY.Priority_RUNTIME, True)
        child.lock('ochild_bool locked')
        self.assertRaises(libdnf5.exception.UserAssertionError,
                          child.set, PRIORITY.Priority_RUNTIME, True)
        self.assertRaises(libdnf5.exception.UserAssertionError,
                          child.set, PRIORITY.Priority_RUNTIME, 'true')

    def test_option_child_string_list(self):
        parent = libdnf5.conf.OptionStringList(('p1', 'p2'))
        child = libdnf5.conf.OptionChildStringList(parent)

        # Child without own value inherits from parent
        self.assertEqual(('p1', 'p2'), child.get_value())
        self.assertEqual(PRIORITY.Priority_DEFAULT, child.get_priority())
        self.assertEqual('', child.get_source())
        self.assertEqual(False, child.has_own_value())

        # Parent set with source - child inherits value, priority, and source
        parent.set(PRIORITY.Priority_RUNTIME, ('a', 'b', 'c'), 'parent.conf')
        self.assertEqual(('a', 'b', 'c'), child.get_value())
        self.assertEqual(PRIORITY.Priority_RUNTIME, child.get_priority())
        self.assertEqual('parent.conf', child.get_source())
        self.assertEqual(False, child.has_own_value())

        # Child set with own value and source (using string parsing)
        child.set(PRIORITY.Priority_COMMANDLINE, 'x, y', 'child.conf')
        self.assertEqual(('x', 'y'), child.get_value())
        self.assertEqual(PRIORITY.Priority_COMMANDLINE, child.get_priority())
        self.assertEqual('child.conf', child.get_source())
        self.assertEqual(True, child.has_own_value())
        # Parent unchanged
        self.assertEqual(('a', 'b', 'c'), parent.get_value())
        self.assertEqual('parent.conf', parent.get_source())

        # Lower priority does not change child value or source
        child.set(PRIORITY.Priority_MAINCONFIG, 'z', 'lower.conf')
        self.assertEqual(('x', 'y'), child.get_value())
        self.assertEqual('child.conf', child.get_source())

        # Child set via string parsing
        child.set(PRIORITY.Priority_RUNTIME, 'm, n', 'parsed.conf')
        self.assertEqual(('m', 'n'), child.get_value())
        self.assertEqual(PRIORITY.Priority_RUNTIME, child.get_priority())
        self.assertEqual('parsed.conf', child.get_source())

        # Locking child
        child.lock('ochild_string_list locked')
        self.assertRaises(libdnf5.exception.UserAssertionError,
                          child.set, PRIORITY.Priority_RUNTIME, 'fail')
