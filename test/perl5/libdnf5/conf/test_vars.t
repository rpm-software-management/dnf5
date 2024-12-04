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

use FindBin;
use lib "$FindBin::Bin/..";  # Add to search path
use BaseTestCase;

use File::Spec::Functions 'catfile';

use libdnf5::conf;


# test_detect_release
{
    my $test = new BaseTestCase();

    my $installroot = $test->{base}->get_config()->get_installroot_option()->get_value();

    # Cannot detect release in nonexistent directory, return undef
    my $release = libdnf5::conf::Vars::detect_release($test->{base}->get_weak_ptr(), catfile($installroot, "nonexist"));
    is($release, undef);
}

done_testing();
