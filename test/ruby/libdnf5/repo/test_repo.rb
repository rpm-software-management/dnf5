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

require 'base_test_case'


class TestRepo < BaseTestCase
    def test_load_system_repo()
        # TODO(lukash) there's no rpmdb in the installroot, create data for the test
        @repo_sack.get_system_repo().load()
    end

    class RepoCallbacks < Repo::RepoCallbacks
        attr_accessor :start_cnt, :start_what
        attr_accessor :end_cnt, :end_error_message
        attr_accessor :progress_cnt, :fastest_mirror_cnt, :handle_mirror_failure_cnt, :repokey_import_cnt

        def initialize()
            super()

            @start_cnt = 0
            @start_what = nil

            @end_cnt = 0
            @end_error_message = nil

            @progress_cnt = 0
            @fastest_mirror_cnt = 0
            @handle_mirror_failure_cnt = 0
            @repokey_import_cnt = 0
        end

        def start(what)
            @start_cnt += 1
            @start_what = what
        end

        def end(error_message)
            @end_cnt += 1
            @end_error_message = error_message
        end

        def progress(total_to_download, downloaded)
            @progress_cnt += 1
            return 0
        end

        def fastest_mirror(stage, ptr)
            @fastest_mirror_cnt += 1
        end

        def handle_mirror_failure(msg, url, metadata)
            @handle_mirror_failure_cnt += 1
            return 0
        end

        def repokey_import(id, user_id, fingerprint, url, timestamp)
            @repokey_import_cnt += 1
            return True
        end
    end

    def test_load_repo()
        repoid = "repomd-repo1"
        repo = add_repo_repomd(repoid, load=false)

        cbs = RepoCallbacks.new()
        repo.set_callbacks(Repo::RepoCallbacksUniquePtr.new(cbs))

        repos = Repo::RepoQuery.new(@base)
        repos.filter_id(repoid)
        @repo_sack.update_and_load_repos(repos)

        assert_equal(1, cbs.start_cnt)
        assert_equal(repoid, cbs.start_what)

        assert_equal(1, cbs.end_cnt)
        assert_equal(nil, cbs.end_error_message)

        assert_operator(1, :<=, cbs.progress_cnt)
        assert_equal(0, cbs.fastest_mirror_cnt)
        assert_equal(0, cbs.handle_mirror_failure_cnt)
        assert_equal(0, cbs.repokey_import_cnt)
    end
end
