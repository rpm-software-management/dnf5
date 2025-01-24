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

require 'libdnf5/base'

class TestBase < Test::Unit::TestCase
    def test_base()
        base = Libdnf5::Base::Base.new()
        logger = base.get_logger()
        config = base.get_config()
        repo_sack = base.get_repo_sack()
        package_sack = base.get_rpm_package_sack()
    end

    def test_weak_ptr()
        # Creates a new Base object
        base = Libdnf5::Base::Base.new()

        # Gets a WeakPtr pointing to Vars in the Base object
        vars = base.get_vars()

        # Creates a copy of WeakPtr
        vars2 = vars

        # Base is valid -> WeakPtr is valid. Sets "test_variable" using WeakPtr vars.
        vars.set("test_variable", "value1")

        # Base is valid -> WeakPtr is valid. Gets value of "test_variable" using copy of WeakPtr vars2.
        assert_equal("value1", vars2.get_value("test_variable"))

        # Invalidates Base object
        base = nil
        # Ensure garbage collection
        GC.start

        # Base object is invalid. -> Both WeakPtr are invalid. The code must throw an exception.
        # Raises an AssertionError that is not caught by the SWIG binding.
        #assert_raise do
        #    vars.get_value("test_variable")
        #end
        #assert_raise do
        #    vars2.get_value("test_variable")
        #end
    end
end
