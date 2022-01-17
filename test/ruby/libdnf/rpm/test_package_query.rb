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


require 'libdnf/base'

require 'base_test_case'


class TestPackageQuery < BaseTestCase
    def setup()
        super
        add_repo_repomd("repomd-repo1")
    end

    def test_size()
        query = Rpm::PackageQuery.new(@base)
        assert_equal(3, query.size(), 'Number of items in the newly created query')
    end

    def test_filter_name()
        # Test QueryCmp::EQ
        query = Rpm::PackageQuery.new(@base)
        query.filter_name(["pkg"])
        assert_equal(1, query.size())

        # TODO(dmach): implement each() so the query can be easily iterated or converted to an array
        actual = []
        it = query.begin()
        while it != query.end()
            actual += [it.value.get_nevra]
            it.next()
        end

        assert_equal(["pkg-1.2-3.x86_64"], actual)

        # ---

        # Test QueryCmp::GLOB
        query = Rpm::PackageQuery.new(@base)
        query.filter_name(["pk*"], Common::QueryCmp_GLOB)
        assert_equal(2, query.size())

        # TODO(dmach): implement each() so the query can be easily iterated or converted to an array
        actual = []
        it = query.begin()
        while it != query.end()
            actual += [it.value.get_nevra]
            it.next()
        end

        assert_equal(["pkg-1.2-3.x86_64", "pkg-libs-1:1.3-4.x86_64"], actual)
    end
end
