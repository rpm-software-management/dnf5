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
