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
        attr_accessor :end_cnt, :end_status, :end_msg
        attr_accessor :progress_cnt, :mirror_failure_cnt

        def initialize()
            super()

            @end_cnt = 0
            @end_status = nil
            @end_msg = nil

            @progress_cnt = 0
            @mirror_failure_cnt = 0
        end

        def end(status, msg)
            @end_cnt += 1
            @end_status = status
            @end_msg = msg
            return 0
        end

        def progress(total_to_download, downloaded)
            @progress_cnt += 1
            return 0
        end

        def mirror_failure(msg, url)
            @mirror_failure_cnt += 1
            return 0
        end
    end

    def test_package_downloader()
        repo = add_repo_rpm("rpm-repo1")

        query = Rpm::PackageQuery.new(@base)
        query.filter_name(["one"])
        query.filter_version(["2"])
        query.filter_arch(["noarch"])
        assert_equal(1, query.size())

        downloader = Repo::PackageDownloader.new()

        cbs = PackageDownloadCallbacks.new()
        downloader.add(query.begin().value, Repo::DownloadCallbacksUniquePtr.new(cbs))

        downloader.download(true, true)

        # forcefully deallocate the downloader, to check cbs is still valid
        downloader = nil
        GC.start

        assert_equal(1, cbs.end_cnt)
        assert_equal(PackageDownloadCallbacks::TransferStatus_SUCCESSFUL, cbs.end_status)
        assert_equal(nil, cbs.end_msg)

        assert_operator(1, :<=, cbs.progress_cnt)
        assert_equal(0, cbs.mirror_failure_cnt)
    end
end
