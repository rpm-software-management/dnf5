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

import unittest
import gc

import libdnf5.base

import base_test_case


class TestPackageDownloader(base_test_case.BaseTestCase):
    def test_package_downloader(self):
        repo = self.add_repo_rpm("rpm-repo1")

        query = libdnf5.rpm.PackageQuery(self.base)
        query.filter_name(["one"])
        query.filter_version(["2"])
        query.filter_arch(["noarch"])
        self.assertEqual(query.size(), 1)

        downloader = libdnf5.repo.PackageDownloader()

        class PackageDownloadCallbacks(libdnf5.repo.DownloadCallbacks):
            end_cnt = 0
            end_status = None
            end_msg = None

            progress_cnt = 0
            mirror_failure_cnt = 0

            def end(self, user_cb_data, status, msg):
                self.end_cnt += 1
                self.end_status = status
                self.end_msg = msg
                return 0

            def progress(self, user_cb_data, total_to_download, downloaded):
                self.progress_cnt += 1
                return 0

            def mirror_failure(self, user_cb_data, msg, url):
                self.mirror_failure_cnt += 1
                return 0

        cbs = PackageDownloadCallbacks()
        self.base.set_download_callbacks(
            libdnf5.repo.DownloadCallbacksUniquePtr(cbs))

        downloader.add(query.begin().value())

        downloader.download(True, True)

        # forcefully deallocate the downloader, to check cbs is still valid
        downloader = None
        gc.collect()

        self.assertEqual(cbs.end_cnt, 1)
        self.assertEqual(
            cbs.end_status, PackageDownloadCallbacks.TransferStatus_SUCCESSFUL)
        self.assertEqual(cbs.end_msg, None)

        self.assertGreaterEqual(cbs.progress_cnt, 1)
        self.assertEqual(cbs.mirror_failure_cnt, 0)
