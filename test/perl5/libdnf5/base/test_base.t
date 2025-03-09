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

use strict;
use warnings;

use Test::More;
use Test::Exception;

use libdnf5::base;
use libdnf5::exception_utils;

{
    # Creates a new Base object
    my $base = new libdnf5::base::Base();
    # Tests returned value
    ok(defined $base, 'new libdnf5::base::Base() returned something');
    ok($base->isa('libdnf5::base::Base'), "  and it's the right class" );

    # Attempts to call some methods from Base object
    my $loger = $base->get_logger();
    my $config = $base->get_config();
    my $repo_sack = $base->get_repo_sack();
    my $package_sack = $base->get_rpm_package_sack();
}

{
    # Tests WeakPtr returned from a Base object.

    # Creates a new Base object
    my $base = new libdnf5::base::Base();

    # Gets a WeakPtr pointing to Vars in the Base object
    my $vars = $base->get_vars();

    # Creates a copy of WeakPtr
    my $vars2 = $vars;

    # Base is valid -> WeakPtr is valid. Sets "test_variable" using WeakPtr vars.
    $vars->set('test_variable', 'value1');

    # Base is valid -> WeakPtr is valid. Gets value of "test_variable" using copy of WeakPtr vars2.
    ok($vars2->get_value('test_variable') eq 'value1');

    # Invalidates Base object
    $base = 0;

    # Base object is invalid. -> Both WeakPtr are invalid. The code must throw an exception.
    # Raises an AssertionError (that is not caught by the SWIG binding).
    throws_ok {
        $vars->get_value('test_variable');
    } 'libdnf5::common::ExceptionWrap';
    my $ex = $@;
    like($ex->stringify(), qr/Dereferencing an invalidated WeakPtr/);

    throws_ok {
        $vars2->get_value('test_variable');
    } 'libdnf5::common::ExceptionWrap';
    my $ex = $@;
    like($ex->stringify(), qr/Dereferencing an invalidated WeakPtr/);
}

{
    # Try to load configuration from non-existing path

    my $base = new libdnf5::base::Base();
    $base->get_config()->get_config_file_path_option()->set('this-path-does-not-exist.conf');

    throws_ok {
        $base->load_config();
    } 'libdnf5::common::ExceptionWrap', 'load_config() throws exception libdnf5::common::ExceptionWrap';
    my $ex = $@;
    like($ex->to_string(), qr/libdnf5::MissingConfigError: Configuration file "this-path-does-not-exist.conf" not found/);
    like($ex->stringify(), qr/Configuration file "this-path-does-not-exist.conf" not found/);

    # Checking the original (wrapped) exception.
    throws_ok {
        libdnf5::exception_utils::rethrow_original($ex);
    } 'libdnf5::conf::MissingConfigError', 'load_config() check original exception';
    my $orig_ex = $@;
    like($orig_ex->stringify(), qr/Configuration file "this-path-does-not-exist.conf" not found/);

    # Checking the nested exception.
    throws_ok {
        libdnf5::exception_utils::rethrow_if_nested($ex);
    } 'libdnf5::common::ExceptionWrap', 'load_config() check nested exception';
    my $nested_ex = $@;
    like($nested_ex->to_string(), qr/libdnf5::utils::fs::FileSystemError: cannot open file/);
    like($nested_ex->stringify(), qr/cannot open file/);

    # Checking the original (wrapped) nested exception.
    throws_ok {
        libdnf5::exception_utils::rethrow_original($nested_ex);
    } 'libdnf5::common::FileSystemError', 'load_config() check original nested exception';
    my $nested_original_ex = $@;
    like($nested_original_ex->stringify(), qr/cannot open file/);
}

done_testing()
