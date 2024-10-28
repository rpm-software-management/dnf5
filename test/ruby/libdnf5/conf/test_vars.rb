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

require 'test/unit'
include Test::Unit::Assertions

require 'libdnf5/conf'

require 'base_test_case'


class TestVars < BaseTestCase
    def test_detect_release()
        installroot = @base.get_config().get_installroot_option().get_value()
        # Cannot detect release in nonexistent directory, return nil
        release = Conf::Vars::detect_release(@base.get_weak_ptr(), File.join(installroot, "nonexist"))
        assert_equal(nil, release)
    end
end
