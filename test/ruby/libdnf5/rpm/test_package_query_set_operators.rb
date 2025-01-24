# Copyright (C) 2021 Red Hat, Inc.
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


require 'libdnf5/base'

require 'base_test_case'


class TestPackageQuerySetOperators < BaseTestCase
    def setup()
        super
        add_repo_solv("solv-24pkgs")
    end

    def test_update()
        # packages with releases: 1, 2
        q1 = Libdnf5::Rpm::PackageQuery.new(@base)
        q1.filter_release(["1", "2"])
        assert_equal(2, q1.size())

        # packages with releases: 2, 3
        q2 = Libdnf5::Rpm::PackageQuery.new(@base)
        q2.filter_release(["2", "3"])
        assert_equal(2, q2.size())

        # union is all 3 packages
        q1.update(q2)
        assert_equal(3, q1.size())
    end

    def test_difference()
        # packages with releases: 1, 2
        q1 = Libdnf5::Rpm::PackageQuery.new(@base)
        q1.filter_release(["1", "2"])
        assert_equal(2, q1.size())

        # packages with releases: 2, 3
        q2 = Libdnf5::Rpm::PackageQuery.new(@base)
        q2.filter_release(["2", "3"])
        assert_equal(2, q2.size())

        # difference is the package with release 1
        q1.difference(q2)
        assert_equal(1, q1.size())
    end

    def test_intersection()
        # packages with releases: 1, 2
        q1 = Libdnf5::Rpm::PackageQuery.new(@base)
        q1.filter_release(["1", "2"])
        assert_equal(2, q1.size())

        # packages with releases: 2, 3
        q2 = Libdnf5::Rpm::PackageQuery.new(@base)
        q2.filter_release(["2", "3"])
        assert_equal(2, q2.size())

        # difference is the package with release 1
        # intersection is the package with release 2
        q1.intersection(q2)
        assert_equal(1, q1.size())
    end
end
