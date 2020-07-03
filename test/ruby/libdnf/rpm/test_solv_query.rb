# Copyright (C) 2020 Red Hat, Inc.
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

require 'test/unit'
include Test::Unit::Assertions

require 'libdnf/base'

class TestSimpleNumber < Test::Unit::TestCase

    def setup()
        @base = Base::Base.new()

        # Sets path to cache directory.
        cwd = Dir.getwd()
        @base.get_config().cachedir().set(Conf::Option::Priority_RUNTIME, cwd)

        @repo_sack = Rpm::RepoSack.new(@base)
        @sack = Rpm::SolvSack.new(@base)

        # Creates new repositories in the repo_sack
        @repo = @repo_sack.new_repo('dnf-ci-fedora')

        # Tunes repositotory configuration (baseurl is mandatory)
        repo_path = File.join(cwd, '../../../test/libdnf/rpm/repos-data/dnf-ci-fedora/')
        baseurl = 'file://' + repo_path
        repo_cfg = @repo.get_config()
        repo_cfg.baseurl().set(Conf::Option::Priority_RUNTIME, baseurl)

        # Loads repository into rpm::Repo.
        @repo.load()

        # Loads rpm::Repo into rpm::SolvSack
        @sack.load_repo(@repo.get(), Rpm::SolvSack::LoadRepoFlags_NONE)
    end

    def teardown()
        ## Nothing really
    end

    def test_size()
        query = Rpm::SolvQuery.new(@sack)
        assert_equal(query.size(), 289, 'Number of items in the newly created query')
    end

    def test_ifilter_name()
        nevras = ['CQRlib-1.1.1-4.fc29.src', 'CQRlib-1.1.1-4.fc29.x86_64']
        nevras_contains = ['CQRlib-1.1.1-4.fc29.src', 'CQRlib-1.1.1-4.fc29.x86_64',
                           'CQRlib-devel-1.1.2-16.fc29.src', 'CQRlib-devel-1.1.2-16.fc29.x86_64']
        full_nevras = ['CQRlib-0:1.1.1-4.fc29.src', 'CQRlib-0:1.1.1-4.fc29.x86_64',
                       'nodejs-1:5.12.1-1.fc29.src', 'nodejs-1:5.12.1-1.fc29.x86_64']

        # Test QueryCmp::EQ
        query = Rpm::SolvQuery.new(@sack)
        names = ["CQRlib"]
        query.ifilter_name(Common::QueryCmp_EQ, Common::VectorString.new(names))
        assert_equal(query.size(), 2)
        pset = query.get_package_set()
        assert_equal(pset.size(), 2)
        it = pset.begin()
        e = pset.end()
        while it != e
            assert(nevras.include?(it.value.get_nevra))
            it.next()
        end

        # Test QueryCmp::GLOB
        query2 = Rpm::SolvQuery.new(@sack)
        names2 = ["CQ?lib"]
        query2.ifilter_name(Common::QueryCmp_GLOB, Common::VectorString.new(names2))
        assert_equal(query2.size(), 2)
        pset2 = query2.get_package_set()
        it = pset2.begin()
        e = pset2.end()
        while it != e
            assert(nevras.include?(it.value.get_nevra))
            it.next()
        end

    end

end
