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


class TestPackageDownloader < BaseTestCase
    class PackageDownloadCallbacks < Repo::DownloadCallbacks
        attr_accessor :user_cb_data_container
        attr_accessor :start_cnt, :progress_cnt, :mirror_failure_cnt, :end_cnt
        attr_accessor :user_cb_data_array, :end_status, :end_msg

        def initialize()
            super()
            @user_cb_data_container = []  # Hold references to user_cb_data
            @start_cnt = 0
            @progress_cnt = 0
            @mirror_failure_cnt = 0
            @end_cnt = 0
            @user_cb_data_array = []
            @end_status = []
            @end_msg = []
        end

        def add_new_download(user_data, description, total_to_download)
            @start_cnt += 1
            user_cb_data = "Package: " + description
            @user_cb_data_container.push(user_cb_data)
            return @user_cb_data_container.length() - 1  # Index of last added element
        end

        def end(user_cb_data, status, msg)
            @end_cnt += 1
            assert(user_cb_data>=0 && user_cb_data <=1, "end: user_cb_data = #{user_cb_data.inspect}")
            @user_cb_data_array.push(user_cb_data)
            @end_status.push(status)
            @end_msg.push(msg)
            return 0
        end

        def progress(user_cb_data, total_to_download, downloaded)
            @progress_cnt += 1
            assert(user_cb_data>=0 && user_cb_data <=1, "progress: user_cb_data = #{user_cb_data.inspect}")
            return 0
        end

        def mirror_failure(user_cb_data, msg, url)
            @mirror_failure_cnt += 1
            assert(user_cb_data>=0 && user_cb_data <=1, "mirror_failure: user_cb_data = #{user_cb_data.inspect}")
            return 0
        end
    end

    def test_package_downloader()
        repo = add_repo_rpm("rpm-repo1")

        query = Rpm::PackageQuery.new(@base)
        query.filter_name(["one"])
        query.filter_arch(["noarch"])
        assert_equal(2, query.size())

        downloader = Repo::PackageDownloader.new(@base)

        cbs = PackageDownloadCallbacks.new()
        @base.set_download_callbacks(Repo::DownloadCallbacksUniquePtr.new(cbs))

        it = query.begin()
        while it != query.end()
            downloader.add(it.value)
            it.next()
        end

        downloader.download()

        # forcefully deallocate the downloader, to check cbs is still valid
        downloader = nil
        GC.start

        assert_equal(["Package: one-0:1-1.noarch", "Package: one-0:2-1.noarch"], [cbs.user_cb_data_container[0], cbs.user_cb_data_container[1]].sort())

        assert_equal(2, cbs.start_cnt)
        assert_operator(2, :<=, cbs.progress_cnt)
        assert_equal(0, cbs.mirror_failure_cnt)
        assert_equal(2, cbs.end_cnt)

        assert_equal([0, 1], cbs.user_cb_data_array.sort())
        assert_equal([PackageDownloadCallbacks::TransferStatus_SUCCESSFUL, PackageDownloadCallbacks::TransferStatus_SUCCESSFUL], cbs.end_status)
        assert_equal([nil, nil], cbs.end_msg)
    end
end
