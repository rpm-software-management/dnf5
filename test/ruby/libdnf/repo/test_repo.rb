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
require 'tmpdir'
include Test::Unit::Assertions

require 'libdnf/base'

class TestRepo < Test::Unit::TestCase
    def test_repo()
        base = Base::Base.new()

        # Sets path to cache directory.
        tmpdir = Dir.mktmpdir("libdnf-ruby-")
        base.get_config().cachedir().set(Conf::Option::Priority_RUNTIME, tmpdir)

        # Sets Base internals according to configuration
        base.setup()

        repo_sack = Repo::RepoSack.new(base)

        # Creates system repository and loads it into rpm::PackageSack.
        repo_sack.get_system_repo().load()

        # Creates new repositories in the repo_sack
        repo = repo_sack.new_repo("repomd-repo1")

        # Tunes repository configuration (baseurl is mandatory)
        repo_path = File.join(Dir.getwd(), '../../../test/data/repos-repomd/repomd-repo1/')
        baseurl = 'file://' + repo_path
        repo_cfg = repo.get_config()
        repo_cfg.baseurl().set(Conf::Option::Priority_RUNTIME, baseurl)

        # fetch repo metadata and load it
        repo.fetch_metadata()
        repo.load()

        # Remove the cache directory.
        FileUtils.remove_entry(tmpdir)
    end
end
