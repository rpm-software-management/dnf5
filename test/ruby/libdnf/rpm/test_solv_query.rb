# Copyright (C) 2020-2021 Red Hat, Inc.
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
        @repo = @repo_sack.new_repo('repomd-repo1')

        # Tunes repository configuration (baseurl is mandatory)
        repo_path = File.join(cwd, '../../../test/data/repos-repomd/repomd-repo1/')
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
        assert_equal(3, query.size(), 'Number of items in the newly created query')
    end

    def test_ifilter_name()
        # Test QueryCmp::EQ
        query = Rpm::SolvQuery.new(@sack)
        query.ifilter_name(Common::QueryCmp_EQ, ["pkg"])
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
        query = Rpm::SolvQuery.new(@sack)
        query.ifilter_name(Common::QueryCmp_GLOB, ["pk*"])
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
