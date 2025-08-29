# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

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
