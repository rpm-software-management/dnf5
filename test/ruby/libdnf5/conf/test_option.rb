# Copyright Contributors to the DNF5 project.
# SPDX-License-Identifier: GPL-2.0-or-later

require 'test/unit'
include Test::Unit::Assertions

require 'libdnf5/conf'

require File.join(File.dirname(__FILE__), '../base_test_case')

P = Libdnf5::Conf::Option

class TestOption < BaseTestCase
    def test_option_bool()
        option = Libdnf5::Conf::OptionBool.new(false)
        assert_equal(false, option.get_default_value())
        assert_equal(false, option.get_value())
        assert_equal(P::Priority_DEFAULT, option.get_priority())
        assert_equal('', option.get_source())

        option.set(P::Priority_COMMANDLINE, 'TrUe', 'cmdline')
        assert_equal(false, option.get_default_value())
        assert_equal(true, option.get_value())
        assert_equal('cmdline', option.get_source())

        # Lower priority does not change value or source
        option.set(P::Priority_MAINCONFIG, false, 'mainconfig')
        assert_equal(true, option.get_value())
        assert_equal('cmdline', option.get_source())

        option.set(P::Priority_RUNTIME, 'FalSe', 'runtime_src')
        assert_equal(false, option.get_value())
        assert_equal(P::Priority_RUNTIME, option.get_priority())
        assert_equal('runtime_src', option.get_source())

        assert_raise(Libdnf5::Exception::OptionInvalidValueError) do
            option.set(P::Priority_RUNTIME, 'invalid')
        end

        option.lock('option locked by test_option_bool')
        assert_raise(Libdnf5::Exception::UserAssertionError) do
            option.set(P::Priority_RUNTIME, true)
        end
        assert_raise(Libdnf5::Exception::UserAssertionError) do
            option.set(true)
        end
    end

    def test_option_enum()
        option = Libdnf5::Conf::OptionEnum.new('bb', ['aa', 'bb', 'cc', 'dd', 'ee'])
        assert_equal('bb', option.get_default_value())
        assert_equal('bb', option.get_value())
        assert_equal(P::Priority_DEFAULT, option.get_priority())
        assert_equal('', option.get_source())

        option.set(P::Priority_RUNTIME, 'cc', 'enum_src')
        assert_equal('cc', option.get_value())
        assert_equal('enum_src', option.get_source())

        assert_raise(Libdnf5::Exception::OptionValueNotAllowedError) do
            option.set(P::Priority_RUNTIME, 'not_allowed')
        end
        # Source unchanged after failed set
        assert_equal('enum_src', option.get_source())

        option.set(P::Priority_RUNTIME, 'aa')
        option.lock('option locked by test_option_enum')
        assert_raise(Libdnf5::Exception::UserAssertionError) do
            option.set(P::Priority_RUNTIME, 'aa')
        end
    end

    def test_option_number()
        option = Libdnf5::Conf::OptionNumberInt32.new(10, -25, 50)
        assert_equal(10, option.get_default_value())
        assert_equal(10, option.get_value())
        assert_equal(P::Priority_DEFAULT, option.get_priority())
        assert_equal('', option.get_source())

        option.set(P::Priority_COMMANDLINE, 5, 'number_src')
        assert_equal(5, option.get_value())
        assert_equal('number_src', option.get_source())

        # Lower priority does not change value or source
        option.set(P::Priority_MAINCONFIG, 3, 'lower_src')
        assert_equal(5, option.get_value())
        assert_equal('number_src', option.get_source())

        assert_raise(Libdnf5::Exception::OptionValueNotAllowedError) do
            option.set(P::Priority_RUNTIME, -26)
        end
        assert_raise(Libdnf5::Exception::OptionValueNotAllowedError) do
            option.set(P::Priority_RUNTIME, 51)
        end

        option.set(P::Priority_RUNTIME, 1)
        option.lock('option locked by test_option_number')
        assert_raise(Libdnf5::Exception::UserAssertionError) do
            option.set(P::Priority_RUNTIME, 1)
        end
    end

    def test_option_path()
        option = Libdnf5::Conf::OptionPath.new('/default_path', false, true)
        assert_equal('/default_path', option.get_default_value())
        assert_equal('/default_path', option.get_value())
        assert_equal(P::Priority_DEFAULT, option.get_priority())
        assert_equal('', option.get_source())

        option.set(P::Priority_RUNTIME, '/path2', 'path_src')
        assert_equal('/path2', option.get_value())
        assert_equal(P::Priority_RUNTIME, option.get_priority())
        assert_equal('path_src', option.get_source())

        assert_raise(Libdnf5::Exception::OptionValueNotAllowedError) do
            option.set(P::Priority_RUNTIME, 'not_absolute')
        end
        # Source unchanged after failed set
        assert_equal('path_src', option.get_source())

        option.lock('option locked by test_option_path')
        assert_raise(Libdnf5::Exception::UserAssertionError) do
            option.set(P::Priority_RUNTIME, '/path2')
        end
    end

    def test_option_string()
        option = Libdnf5::Conf::OptionString.new('default', 'd.*t', false)
        assert_equal('default', option.get_default_value())
        assert_equal('default', option.get_value())
        assert_equal(P::Priority_DEFAULT, option.get_priority())
        assert_equal('', option.get_source())

        option.set(P::Priority_COMMANDLINE, 'donut', 'string_src')
        assert_equal('default', option.get_default_value())
        assert_equal('donut', option.get_value())
        assert_equal('string_src', option.get_source())

        # Case sensitive - uppercase 'T' is not allowed
        assert_raise(Libdnf5::Exception::OptionValueNotAllowedError) do
            option.set(P::Priority_RUNTIME, 'donuT')
        end
        # Source unchanged after failed set
        assert_equal('string_src', option.get_source())

        option.lock('option locked by test_option_string')
        assert_raise(Libdnf5::Exception::UserAssertionError) do
            option.set(P::Priority_RUNTIME, 'doXXnut')
        end
    end

    def test_option_string_list()
        option = Libdnf5::Conf::OptionStringList.new(['dval1X', 'dval2X'], '[d|c].*X', false)
        assert_equal(['dval1X', 'dval2X'], option.get_default_value())
        assert_equal(['dval1X', 'dval2X'], option.get_value())
        assert_equal(P::Priority_DEFAULT, option.get_priority())
        assert_equal('', option.get_source())

        option.set(P::Priority_RUNTIME, ['donutX', 'cakeX'], 'list_src')
        assert_equal(['dval1X', 'dval2X'], option.get_default_value())
        assert_equal(['donutX', 'cakeX'], option.get_value())
        assert_equal(P::Priority_RUNTIME, option.get_priority())
        assert_equal('list_src', option.get_source())

        # Case sensitive - lowercase 'x' is not allowed
        assert_raise(Libdnf5::Exception::OptionValueNotAllowedError) do
            option.set(P::Priority_RUNTIME, ['donutX', 'cakex'])
        end
        # Source unchanged after failed set
        assert_equal('list_src', option.get_source())

        option.set(P::Priority_RUNTIME, 'dfirstX, dsecondX', 'string_list_src')
        assert_equal(['dfirstX', 'dsecondX'], option.get_value())
        assert_equal('string_list_src', option.get_source())

        option.lock('option locked by test_option_string_list')
        assert_raise(Libdnf5::Exception::UserAssertionError) do
            option.set(P::Priority_RUNTIME, 'doXXnut')
        end
    end

    def test_option_string_set()
        option = Libdnf5::Conf::OptionStringSet.new(['x', 'y', 'z'])
        assert_equal('', option.get_source())

        option.set(P::Priority_RUNTIME, 'a, b, c', 'set_src')
        assert_equal('set_src', option.get_source())
    end

    def test_option_list_add()
        option = Libdnf5::Conf::OptionStringSet.new('1, 2, 3')

        option.add(P::Priority_RUNTIME, ['4', '5', '6'], 'add_src')
        assert_equal(['1', '2', '3', '4', '5', '6'], option.get_value().sort)
        assert_equal('add_src', option.get_source())
    end

    def test_option_string_append_list()
        option = Libdnf5::Conf::OptionStringAppendList.new('Pkg1, Pkg2')
        assert_equal(['Pkg1', 'Pkg2'], option.get_value())
        assert_equal('', option.get_source())

        # setting a new value will append to current value
        option.set(P::Priority_COMMANDLINE, 'Pkg3', 'cmdline_src')
        assert_equal(['Pkg1', 'Pkg2', 'Pkg3'], option.get_value())
        assert_equal('cmdline_src', option.get_source())

        # values are evaluated ordered by the priority
        option.set(P::Priority_MAINCONFIG, 'Pkg4', 'mainconfig_src')
        assert_equal(['Pkg1', 'Pkg2', 'Pkg4', 'Pkg3'], option.get_value())
        # COMMANDLINE has higher priority, source stays from COMMANDLINE
        assert_equal('cmdline_src', option.get_source())

        # I can clear the option using empty value
        option.set(P::Priority_COMMANDLINE, '', 'clear_src')
        assert_equal([], option.get_value())
        assert_equal('clear_src', option.get_source())
    end

    def test_option_child_bool()
        parent = Libdnf5::Conf::OptionBool.new(true)
        child = Libdnf5::Conf::OptionChildBool.new(parent)
        assert_equal(true, child.get_default_value())
        assert_equal(true, child.get_value())
        assert_equal(P::Priority_DEFAULT, child.get_priority())
        assert_equal('', child.get_source())

        parent.set(P::Priority_RUNTIME, false, 'parent_src')
        assert_equal(true, child.get_default_value())
        assert_equal(false, child.get_value())
        assert_equal(P::Priority_RUNTIME, child.get_priority())
        assert_equal('parent_src', child.get_source())

        child.set(P::Priority_COMMANDLINE, false, 'child_src')
        assert_equal(true, child.get_default_value())
        assert_equal(false, child.get_value())
        assert_equal(P::Priority_COMMANDLINE, child.get_priority())
        assert_equal('child_src', child.get_source())

        # Lower priority does not change value or source
        child.set(P::Priority_MAINCONFIG, true, 'lower_src')
        assert_equal(false, child.get_value())
        assert_equal(P::Priority_COMMANDLINE, child.get_priority())
        assert_equal('child_src', child.get_source())

        child.set(P::Priority_COMMANDLINE, true)
        assert_equal(true, child.get_value())

        assert_raise(Libdnf5::Exception::OptionInvalidValueError) do
            child.set(P::Priority_COMMANDLINE, 'invalid')
        end

        child.set(P::Priority_RUNTIME, true)
        child.lock('ochild_bool locked')
        assert_raise(Libdnf5::Exception::UserAssertionError) do
            child.set(P::Priority_RUNTIME, true)
        end
        assert_raise(Libdnf5::Exception::UserAssertionError) do
            child.set(P::Priority_RUNTIME, 'true')
        end
    end

    def test_option_child_string_list()
        parent = Libdnf5::Conf::OptionStringList.new(['p1', 'p2'])
        child = Libdnf5::Conf::OptionChildStringList.new(parent)

        # Child without own value inherits from parent
        assert_equal(['p1', 'p2'], child.get_value())
        assert_equal(P::Priority_DEFAULT, child.get_priority())
        assert_equal('', child.get_source())

        # Parent set with source - child inherits value, priority, and source
        parent.set(P::Priority_RUNTIME, ['a', 'b', 'c'], 'parent.conf')
        assert_equal(['a', 'b', 'c'], child.get_value())
        assert_equal(P::Priority_RUNTIME, child.get_priority())
        assert_equal('parent.conf', child.get_source())

        # Child set with own value and source (using string parsing)
        child.set(P::Priority_COMMANDLINE, 'x, y', 'child.conf')
        assert_equal(['x', 'y'], child.get_value())
        assert_equal(P::Priority_COMMANDLINE, child.get_priority())
        assert_equal('child.conf', child.get_source())
        # Parent unchanged
        assert_equal(['a', 'b', 'c'], parent.get_value())
        assert_equal('parent.conf', parent.get_source())

        # Lower priority does not change child value or source
        child.set(P::Priority_MAINCONFIG, 'z', 'lower.conf')
        assert_equal(['x', 'y'], child.get_value())
        assert_equal('child.conf', child.get_source())

        # Child set via string parsing
        child.set(P::Priority_RUNTIME, 'm, n', 'parsed.conf')
        assert_equal(['m', 'n'], child.get_value())
        assert_equal(P::Priority_RUNTIME, child.get_priority())
        assert_equal('parsed.conf', child.get_source())

        # Locking child
        child.lock('ochild_string_list locked')
        assert_raise(Libdnf5::Exception::UserAssertionError) do
            child.set(P::Priority_RUNTIME, 'fail')
        end
    end
end
