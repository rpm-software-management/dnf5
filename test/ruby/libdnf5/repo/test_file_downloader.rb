# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later
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


class TestFileDownloader < BaseTestCase
    USER_DATA = 25

    class DownloadCallbacks < Libdnf5::Repo::DownloadCallbacks
        attr_accessor :start_cnt, :progress_cnt, :mirror_failure_cnt, :end_cnt
        attr_accessor :end_status, :end_msg

        USER_CB_DATA = 5

        def initialize()
            super()
            @start_cnt = 0
            @progress_cnt = 0
            @mirror_failure_cnt = 0
            @end_cnt = 0
            @end_status = nil
            @end_msg = nil
        end

        def add_new_download(user_data, description, total_to_download)
            @start_cnt += 1
            assert_equal(USER_DATA, user_data)
            return USER_CB_DATA
        end

        def end(user_cb_data, status, msg)
            @end_cnt += 1
            assert_equal(USER_CB_DATA, user_cb_data)
            @end_status = status
            @end_msg = msg
            return 0
        end

        def progress(user_cb_data, total_to_download, downloaded)
            @progress_cnt += 1
            assert_equal(USER_CB_DATA, user_cb_data)
            return 0
        end

        def mirror_failure(user_cb_data, msg, url, metadata)
            @mirror_failure_cnt += 1
            assert_equal(USER_CB_DATA, user_cb_data)
            return 0
        end
    end

    def test_file_downloader()
        source_file_path = File.join(PROJECT_SOURCE_DIR, "test/data/keys/key.pub")
        source_url = "file://" + source_file_path
        dest_file_path = File.join(@temp_dir, "file_downloader.pub")

        dl_cbs = DownloadCallbacks.new()
        @base.set_download_callbacks(Libdnf5::Repo::DownloadCallbacksUniquePtr.new(dl_cbs))

        file_downloader = Libdnf5::Repo::FileDownloader.new(@base)
        file_downloader.add(source_url, dest_file_path, USER_DATA)
        file_downloader.download()

        assert_equal(1, dl_cbs.start_cnt)
        assert_operator(1, :<=, dl_cbs.progress_cnt)
        assert_equal(0, dl_cbs.mirror_failure_cnt)
        assert_equal(1, dl_cbs.end_cnt)

        assert_equal(DownloadCallbacks::TransferStatus_SUCCESSFUL, dl_cbs.end_status)
        assert_equal(nil, dl_cbs.end_msg)
    end
end
