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

require 'libdnf/base'

require 'base_test_case'


class TestRepo < BaseTestCase
    def test_load_system_repo()
        # TODO(lukash) there's no rpmdb in the installroot, create data for the test
        @repo_sack.get_system_repo().load()
    end

    def test_repo()
        repo = add_repo_repomd("repomd-repo1", load=false)

        repo.fetch_metadata()
        repo.load()
    end
end
