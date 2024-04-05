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
require 'libdnf5/repo'

require 'base_test_case'


class TestRepo < BaseTestCase
    class DownloadCallbacks < Repo::DownloadCallbacks
        attr_accessor :start_cnt, :start_what
        attr_accessor :end_cnt, :end_error_message
        attr_accessor :progress_cnt, :fastest_mirror_cnt, :handle_mirror_failure_cnt

        def initialize()
            super()

            @start_cnt = 0
            @start_what = nil

            @end_cnt = 0
            @end_error_message = nil

            @progress_cnt = 0
            @fastest_mirror_cnt = 0
            @handle_mirror_failure_cnt = 0
        end

        def add_new_download(user_data, descr, total)
            @start_cnt += 1
            @start_what = descr
            return nil
        end

        def end(user_cb_data, status, error_message)
            @end_cnt += 1
            @end_error_message = error_message
            return 0
        end

        def progress(user_cb_data, total_to_download, downloaded)
            @progress_cnt += 1
            return 0
        end

        def fastest_mirror(user_cb_data, stage, ptr)
            @fastest_mirror_cnt += 1
        end

        def handle_mirror_failure(user_cb_data, msg, url, metadata)
            @handle_mirror_failure_cnt += 1
            return 0
        end
    end

    class RepoCallbacks < Repo::RepoCallbacks
        attr_accessor :repokey_import_cnt

        def initialize()
            super()

            @repokey_import_cnt = 0
        end

        def repokey_import(id, user_id, fingerprint, url, timestamp)
            @repokey_import_cnt += 1
            return True
        end
    end

    def test_load_repo()
        repoid = "repomd-repo1"
        repo = add_repo_repomd(repoid, load=false)

        dl_cbs = DownloadCallbacks.new()
        @base.set_download_callbacks(Repo::DownloadCallbacksUniquePtr.new(dl_cbs))

        cbs = RepoCallbacks.new()
        repo.set_callbacks(Repo::RepoCallbacksUniquePtr.new(cbs))

        @repo_sack.load_repos(Repo::Repo::Type_AVAILABLE)

        assert_equal(1, dl_cbs.start_cnt)
        assert_equal(repoid, dl_cbs.start_what)

        assert_equal(1, dl_cbs.end_cnt)
        assert_equal(nil, dl_cbs.end_error_message)

        assert_operator(1, :<=, dl_cbs.progress_cnt)
        assert_equal(0, dl_cbs.fastest_mirror_cnt)
        assert_equal(0, dl_cbs.handle_mirror_failure_cnt)
        assert_equal(0, cbs.repokey_import_cnt)
    end
end
