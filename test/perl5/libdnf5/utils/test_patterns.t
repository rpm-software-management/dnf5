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

use strict;
use warnings;

use Test::More;

use libdnf5::utils;

{
    ok(!libdnf5::utils::is_file_pattern(''), 'is_file_pattern ""');
    ok(!libdnf5::utils::is_file_pattern('no_file_pattern'), 'is_file_pattern "no_file_pattern"');
    ok(!libdnf5::utils::is_file_pattern('no_file/pattern'), 'is_file_pattern "no_file/pattern"');
    ok(libdnf5::utils::is_file_pattern('/pattern') == 1, 'is_file_pattern "/pattern"');
    ok(libdnf5::utils::is_file_pattern('*/pattern') == 1, 'is_file_pattern "*/pattern"');
}

{
    ok(!libdnf5::utils::is_glob_pattern(''), 'is_glob_pattern ""');
    ok(!libdnf5::utils::is_glob_pattern('no_glob_pattern'), 'is_glob_pattern "no_glob_pattern"');
    ok(libdnf5::utils::is_glob_pattern('glob*_pattern'), 'is_glob_pattern "glob*_pattern"');
    ok(libdnf5::utils::is_glob_pattern('glob[sdf]_pattern') == 1, 'is_glob_pattern "glob[sdf]_pattern"');
    ok(libdnf5::utils::is_glob_pattern('glob?_pattern') == 1, 'is_glob_pattern "glob?_pattern"');
}

done_testing()
