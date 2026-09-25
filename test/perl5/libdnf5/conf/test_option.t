# Copyright Contributors to the DNF5 project.
# SPDX-License-Identifier: GPL-2.0-or-later

use strict;
use warnings;

use Test::More;
use Test::Exception;

use FindBin;
use lib "$FindBin::Bin/..";
use BaseTestCase;

use libdnf5::conf;
use libdnf5::common;

my $DEFAULT   = $libdnf5::conf::Option::Priority_DEFAULT;
my $MAINCONFIG = $libdnf5::conf::Option::Priority_MAINCONFIG;
my $COMMANDLINE = $libdnf5::conf::Option::Priority_COMMANDLINE;
my $RUNTIME   = $libdnf5::conf::Option::Priority_RUNTIME;

# Converts SWIG VectorString or Perl arrayref to a Perl arrayref for comparison
sub to_list {
    my ($value) = @_;
    return $value if ref($value) eq 'ARRAY';
    my @result;
    for (my $i = 0; $i < $value->size(); $i++) {
        push @result, $value->get($i);
    }
    return \@result;
}

# test_option_bool
{
    my $option = new libdnf5::conf::OptionBool(0);
    ok(!$option->get_default_value(), 'bool: default value is false');
    ok(!$option->get_value(), 'bool: value is false');
    is($option->get_priority(), $DEFAULT, 'bool: default priority');
    is($option->get_source(), '', 'bool: default source is empty');

    $option->set($COMMANDLINE, 'TrUe', 'cmdline');
    ok(!$option->get_default_value(), 'bool: default value unchanged');
    ok($option->get_value(), 'bool: value set to true');
    is($option->get_source(), 'cmdline', 'bool: source set');

    # Lower priority does not change value or source
    $option->set($MAINCONFIG, 0, 'mainconfig');
    ok($option->get_value(), 'bool: value unchanged with lower priority');
    is($option->get_source(), 'cmdline', 'bool: source unchanged with lower priority');

    $option->set($RUNTIME, 'FalSe', 'runtime_src');
    ok(!$option->get_value(), 'bool: value set to false');
    is($option->get_priority(), $RUNTIME, 'bool: runtime priority');
    is($option->get_source(), 'runtime_src', 'bool: source updated');

    throws_ok {
        $option->set($RUNTIME, 'invalid');
    } 'libdnf5::exception::OptionInvalidValueError', 'bool: invalid value throws';

    $option->lock('option locked by test_option_bool');
    throws_ok {
        $option->set($RUNTIME, 1);
    } 'libdnf5::exception::UserAssertionError', 'bool: writing to locked option throws';
    throws_ok {
        $option->set(1);
    } 'libdnf5::exception::UserAssertionError', 'bool: writing to locked option throws (no priority)';
}

# test_option_enum
{
    my $option = new libdnf5::conf::OptionEnum('bb', ['aa', 'bb', 'cc', 'dd', 'ee']);
    is($option->get_default_value(), 'bb', 'enum: default value');
    is($option->get_value(), 'bb', 'enum: value');
    is($option->get_priority(), $DEFAULT, 'enum: default priority');
    is($option->get_source(), '', 'enum: default source is empty');

    $option->set($RUNTIME, 'cc', 'enum_src');
    is($option->get_value(), 'cc', 'enum: value set');
    is($option->get_source(), 'enum_src', 'enum: source set');

    throws_ok {
        $option->set($RUNTIME, 'not_allowed');
    } 'libdnf5::exception::OptionValueNotAllowedError', 'enum: not allowed value throws';
    # Source unchanged after failed set
    is($option->get_source(), 'enum_src', 'enum: source unchanged after failed set');

    $option->set($RUNTIME, 'aa');
    $option->lock('option locked by test_option_enum');
    throws_ok {
        $option->set($RUNTIME, 'aa');
    } 'libdnf5::exception::UserAssertionError', 'enum: writing to locked option throws';
}

# test_option_number
{
    my $option = new libdnf5::conf::OptionNumberInt32(10, -25, 50);
    is($option->get_default_value(), 10, 'number: default value');
    is($option->get_value(), 10, 'number: value');
    is($option->get_priority(), $DEFAULT, 'number: default priority');
    is($option->get_source(), '', 'number: default source is empty');

    $option->set($COMMANDLINE, 5, 'number_src');
    is($option->get_value(), 5, 'number: value set');
    is($option->get_source(), 'number_src', 'number: source set');

    # Lower priority does not change value or source
    $option->set($MAINCONFIG, 3, 'lower_src');
    is($option->get_value(), 5, 'number: value unchanged with lower priority');
    is($option->get_source(), 'number_src', 'number: source unchanged with lower priority');

    throws_ok {
        $option->set($RUNTIME, -26);
    } 'libdnf5::exception::OptionValueNotAllowedError', 'number: below min throws';
    throws_ok {
        $option->set($RUNTIME, 51);
    } 'libdnf5::exception::OptionValueNotAllowedError', 'number: above max throws';

    $option->set($RUNTIME, 1);
    $option->lock('option locked by test_option_number');
    throws_ok {
        $option->set($RUNTIME, 1);
    } 'libdnf5::exception::UserAssertionError', 'number: writing to locked option throws';
}

# test_option_path
{
    my $option = new libdnf5::conf::OptionPath('/default_path', 0, 1);
    is($option->get_default_value(), '/default_path', 'path: default value');
    is($option->get_value(), '/default_path', 'path: value');
    is($option->get_priority(), $DEFAULT, 'path: default priority');
    is($option->get_source(), '', 'path: default source is empty');

    $option->set($RUNTIME, '/path2', 'path_src');
    is($option->get_value(), '/path2', 'path: value set');
    is($option->get_priority(), $RUNTIME, 'path: runtime priority');
    is($option->get_source(), 'path_src', 'path: source set');

    throws_ok {
        $option->set($RUNTIME, 'not_absolute');
    } 'libdnf5::exception::OptionValueNotAllowedError', 'path: not absolute throws';
    # Source unchanged after failed set
    is($option->get_source(), 'path_src', 'path: source unchanged after failed set');

    $option->lock('option locked by test_option_path');
    throws_ok {
        $option->set($RUNTIME, '/path2');
    } 'libdnf5::exception::UserAssertionError', 'path: writing to locked option throws';
}

# test_option_string
{
    my $option = new libdnf5::conf::OptionString('default', 'd.*t', 0);
    is($option->get_default_value(), 'default', 'string: default value');
    is($option->get_value(), 'default', 'string: value');
    is($option->get_priority(), $DEFAULT, 'string: default priority');
    is($option->get_source(), '', 'string: default source is empty');

    $option->set($COMMANDLINE, 'donut', 'string_src');
    is($option->get_default_value(), 'default', 'string: default value unchanged');
    is($option->get_value(), 'donut', 'string: value set');
    is($option->get_source(), 'string_src', 'string: source set');

    # Case sensitive - uppercase 'T' is not allowed
    throws_ok {
        $option->set($RUNTIME, 'donuT');
    } 'libdnf5::exception::OptionValueNotAllowedError', 'string: case sensitive throws';
    # Source unchanged after failed set
    is($option->get_source(), 'string_src', 'string: source unchanged after failed set');

    $option->lock('option locked by test_option_string');
    throws_ok {
        $option->set($RUNTIME, 'doXXnut');
    } 'libdnf5::exception::UserAssertionError', 'string: writing to locked option throws';
}

# test_option_string_list
{
    my $option = new libdnf5::conf::OptionStringList('dval1X, dval2X', '[d|c].*X', 0);
    is_deeply(to_list($option->get_default_value()), ['dval1X', 'dval2X'], 'string_list: default value');
    is_deeply(to_list($option->get_value()), ['dval1X', 'dval2X'], 'string_list: value');
    is($option->get_priority(), $DEFAULT, 'string_list: default priority');
    is($option->get_source(), '', 'string_list: default source is empty');

    $option->set($RUNTIME, 'donutX, cakeX', 'list_src');
    is_deeply(to_list($option->get_default_value()), ['dval1X', 'dval2X'], 'string_list: default value unchanged');
    is_deeply(to_list($option->get_value()), ['donutX', 'cakeX'], 'string_list: value set');
    is($option->get_priority(), $RUNTIME, 'string_list: runtime priority');
    is($option->get_source(), 'list_src', 'string_list: source set');

    # Case sensitive - lowercase 'x' is not allowed
    throws_ok {
        $option->set($RUNTIME, 'donutX, cakex');
    } 'libdnf5::exception::OptionValueNotAllowedError', 'string_list: case sensitive throws';
    # Source unchanged after failed set
    is($option->get_source(), 'list_src', 'string_list: source unchanged after failed set');

    $option->set($RUNTIME, 'dfirstX, dsecondX', 'string_list_src');
    is_deeply(to_list($option->get_value()), ['dfirstX', 'dsecondX'], 'string_list: value set from string');
    is($option->get_source(), 'string_list_src', 'string_list: source set from string');

    $option->lock('option locked by test_option_string_list');
    throws_ok {
        $option->set($RUNTIME, 'doXXnut');
    } 'libdnf5::exception::UserAssertionError', 'string_list: writing to locked option throws';
}

# test_option_string_set
{
    my $option = new libdnf5::conf::OptionStringSet('x, y, z');
    is($option->get_source(), '', 'string_set: default source is empty');

    $option->set($RUNTIME, 'a, b, c', 'set_src');
    is($option->get_source(), 'set_src', 'string_set: source set');
}

# test_option_list_add
{
    my $option = new libdnf5::conf::OptionStringSet('1, 2, 3');

    $option->add($RUNTIME, '4, 5, 6', 'add_src');
    is($option->get_value_string(), '1,2,3,4,5,6', 'list_add: values added');
    is($option->get_source(), 'add_src', 'list_add: source set');
}

# test_option_string_append_list
{
    my $option = new libdnf5::conf::OptionStringAppendList('Pkg1, Pkg2');
    is_deeply(to_list($option->get_value()), ['Pkg1', 'Pkg2'], 'append_list: default value');
    is($option->get_source(), '', 'append_list: default source is empty');

    # setting a new value will append to current value
    $option->set($COMMANDLINE, 'Pkg3', 'cmdline_src');
    is_deeply(to_list($option->get_value()), ['Pkg1', 'Pkg2', 'Pkg3'], 'append_list: value appended');
    is($option->get_source(), 'cmdline_src', 'append_list: source set');

    # values are evaluated ordered by the priority
    $option->set($MAINCONFIG, 'Pkg4', 'mainconfig_src');
    is_deeply(to_list($option->get_value()), ['Pkg1', 'Pkg2', 'Pkg4', 'Pkg3'], 'append_list: ordered by priority');
    # COMMANDLINE has higher priority, source stays from COMMANDLINE
    is($option->get_source(), 'cmdline_src', 'append_list: source from higher priority');

    # I can clear the option using empty value
    $option->set($COMMANDLINE, '', 'clear_src');
    is_deeply(to_list($option->get_value()), [], 'append_list: cleared');
    is($option->get_source(), 'clear_src', 'append_list: clear source set');
}

# test_option_child_bool
{
    my $parent = new libdnf5::conf::OptionBool(1);
    my $child = new libdnf5::conf::OptionChildBool($parent);
    ok($child->get_default_value(), 'child_bool: default value');
    ok($child->get_value(), 'child_bool: value');
    is($child->get_priority(), $DEFAULT, 'child_bool: default priority');
    is($child->get_source(), '', 'child_bool: default source is empty');
    ok(!$child->has_own_value(), 'child_bool: no own value initially');

    $parent->set($RUNTIME, 0, 'parent_src');
    ok($child->get_default_value(), 'child_bool: default value unchanged');
    ok(!$child->get_value(), 'child_bool: inherits parent value');
    is($child->get_priority(), $RUNTIME, 'child_bool: inherits parent priority');
    is($child->get_source(), 'parent_src', 'child_bool: inherits parent source');
    ok(!$child->has_own_value(), 'child_bool: still no own value after parent set');

    $child->set($COMMANDLINE, 0, 'child_src');
    ok($child->get_default_value(), 'child_bool: default value unchanged');
    ok(!$child->get_value(), 'child_bool: own value');
    is($child->get_priority(), $COMMANDLINE, 'child_bool: own priority');
    is($child->get_source(), 'child_src', 'child_bool: own source');
    ok($child->has_own_value(), 'child_bool: has own value after child set');

    # Lower priority does not change value or source
    $child->set($MAINCONFIG, 1, 'lower_src');
    ok(!$child->get_value(), 'child_bool: value unchanged with lower priority');
    is($child->get_priority(), $COMMANDLINE, 'child_bool: priority unchanged with lower priority');
    is($child->get_source(), 'child_src', 'child_bool: source unchanged with lower priority');

    $child->set($COMMANDLINE, 1);
    ok($child->get_value(), 'child_bool: value set to true');

    throws_ok {
        $child->set($COMMANDLINE, 'invalid');
    } 'libdnf5::exception::OptionInvalidValueError', 'child_bool: invalid value throws';

    $child->set($RUNTIME, 1);
    $child->lock('ochild_bool locked');
    throws_ok {
        $child->set($RUNTIME, 1);
    } 'libdnf5::exception::UserAssertionError', 'child_bool: writing to locked throws';
    throws_ok {
        $child->set($RUNTIME, 'true');
    } 'libdnf5::exception::UserAssertionError', 'child_bool: writing string to locked throws';
}

# test_option_child_string_list
{
    my $parent = new libdnf5::conf::OptionStringList('p1, p2');
    my $child = new libdnf5::conf::OptionChildStringList($parent);

    # Child without own value inherits from parent
    is_deeply(to_list($child->get_value()), ['p1', 'p2'], 'child_list: inherits parent value');
    is($child->get_priority(), $DEFAULT, 'child_list: inherits parent priority');
    is($child->get_source(), '', 'child_list: default source is empty');
    ok(!$child->has_own_value(), 'child_list: no own value initially');

    # Parent set with source - child inherits value, priority, and source
    $parent->set($RUNTIME, 'a, b, c', 'parent.conf');
    is_deeply(to_list($child->get_value()), ['a', 'b', 'c'], 'child_list: inherits parent value after set');
    is($child->get_priority(), $RUNTIME, 'child_list: inherits parent priority after set');
    is($child->get_source(), 'parent.conf', 'child_list: inherits parent source');
    ok(!$child->has_own_value(), 'child_list: still no own value after parent set');

    # Child set with own value and source (using string parsing)
    $child->set($COMMANDLINE, 'x, y', 'child.conf');
    is_deeply(to_list($child->get_value()), ['x', 'y'], 'child_list: own value');
    is($child->get_priority(), $COMMANDLINE, 'child_list: own priority');
    is($child->get_source(), 'child.conf', 'child_list: own source');
    ok($child->has_own_value(), 'child_list: has own value after child set');
    # Parent unchanged
    is_deeply(to_list($parent->get_value()), ['a', 'b', 'c'], 'child_list: parent value unchanged');
    is($parent->get_source(), 'parent.conf', 'child_list: parent source unchanged');

    # Lower priority does not change child value or source
    $child->set($MAINCONFIG, 'z', 'lower.conf');
    is_deeply(to_list($child->get_value()), ['x', 'y'], 'child_list: value unchanged with lower priority');
    is($child->get_source(), 'child.conf', 'child_list: source unchanged with lower priority');

    # Child set via string parsing
    $child->set($RUNTIME, 'm, n', 'parsed.conf');
    is_deeply(to_list($child->get_value()), ['m', 'n'], 'child_list: value set via string parsing');
    is($child->get_priority(), $RUNTIME, 'child_list: priority after string parsing');
    is($child->get_source(), 'parsed.conf', 'child_list: source after string parsing');

    # Locking child
    $child->lock('ochild_string_list locked');
    throws_ok {
        $child->set($RUNTIME, 'fail');
    } 'libdnf5::exception::UserAssertionError', 'child_list: writing to locked throws';
}

done_testing()
