# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

require 'test/unit'
include Test::Unit::Assertions

require 'libdnf5/utils'

class TestIsXPattern < Test::Unit::TestCase
    def test_is_file_pattern()
        assert(!Libdnf5::Utils::is_file_pattern(''))
        assert(!Libdnf5::Utils::is_file_pattern('no_file_pattern'))
        assert(!Libdnf5::Utils::is_file_pattern('no_file/pattern'))
        assert(Libdnf5::Utils::is_file_pattern('/pattern'))
        assert(Libdnf5::Utils::is_file_pattern('*/pattern'))
    end

    def test_is_glob_pattern()
        assert(!Libdnf5::Utils::is_glob_pattern(''))
        assert(!Libdnf5::Utils::is_glob_pattern('no_glob_pattern'))
        assert(Libdnf5::Utils::is_glob_pattern('glob*_pattern'))
        assert(Libdnf5::Utils::is_glob_pattern('glob[sdf]_pattern'))
        assert(Libdnf5::Utils::is_glob_pattern('glob?_pattern'))
    end
end
