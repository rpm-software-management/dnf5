# Copyright Contributors to the DNF5 project.
# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later


require 'libdnf5/base'

require 'base_test_case'


class TestPackageQuery < BaseTestCase
    def setup()
        super
        add_repo_repomd("repomd-repo1")
    end

    def test_size()
        query = Libdnf5::Rpm::PackageQuery.new(@base)
        assert_equal(3, query.size(), 'Number of items in the newly created query')
    end

    def test_filter_name()
        # Test QueryCmp::EQ
        query = Libdnf5::Rpm::PackageQuery.new(@base)
        query.filter_name(["pkg"])
        assert_equal(1, query.size())

        actual = []
        it = query.begin()
        while it != query.end()
            actual += [it.value.get_nevra]
            it.next()
        end

        assert_equal(["pkg-1.2-3.x86_64"], actual)

        # ---

        # Test QueryCmp::GLOB
        query = Libdnf5::Rpm::PackageQuery.new(@base)
        query.filter_name(["pk*"], Libdnf5::Common::QueryCmp_GLOB)
        assert_equal(2, query.size())

        actual = []
        it = query.begin()
        while it != query.end()
            actual += [it.value.get_nevra]
            it.next()
        end

        assert_equal(["pkg-1.2-3.x86_64", "pkg-libs-1:1.3-4.x86_64"], actual)
    end

    def test_implements_enumerable()
        query = Libdnf5::Rpm::PackageQuery.new(@base)
        query.filter_name(["pkg"])
        assert_equal(1, query.size())

        # Using each() without a block should return Enumerator.
        assert_instance_of(Enumerator, query.each)

        # Using each() with a block should return the collection.
        assert_instance_of(Libdnf5::Rpm::PackageSet, query.each(&:get_name))

        actual_nevra = query.map { |pkg| pkg.get_nevra }

        assert_equal(["pkg-1.2-3.x86_64"], actual_nevra)

        # ---

        query = Libdnf5::Rpm::PackageQuery.new(@base)
        query.filter_name(["pk*"], Libdnf5::Common::QueryCmp_GLOB)
        assert_equal(2, query.size())

        # Test other method than each that comes with Enumerable
        actual = query.select { |pkg| pkg.get_name == "pkg-libs" }

        assert_equal(1, actual.size)
        assert_equal('pkg-libs-1:1.3-4.x86_64', actual.first.get_nevra)
    end
end
