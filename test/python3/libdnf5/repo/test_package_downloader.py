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
        class PackageDownloadCallbacks(libdnf5.repo.DownloadCallbacks):
            test_instance = None

            def __init__(self):
                super(PackageDownloadCallbacks, self).__init__()
                self.user_cb_data_container = []  # Hold references to user_cb_data
                self.start_cnt = 0
                self.progress_cnt = 0
                self.mirror_failure_cnt = 0
                self.end_cnt = 0
                self.user_data_array = []
                self.user_cb_data_array = []
                self.end_status = []
                self.end_msg = []

            def add_new_download(self, user_data, description, total_to_download):
                self.start_cnt += 1
                self.user_data_array.append(user_data)
                user_cb_data_reference = "Package: " + description
                self.user_cb_data_container.append(user_cb_data_reference)
                return len(self.user_cb_data_container) - 1

            def end(self, user_cb_data, status, msg):
                self.end_cnt += 1
                self.test_instance.assertIn(user_cb_data, [0, 1])
                self.user_cb_data_array.append(user_cb_data)
                self.end_status.append(status)
                self.end_msg.append(msg)
                return 0

            def progress(self, user_cb_data, total_to_download, downloaded):
                self.progress_cnt += 1
                self.test_instance.assertIn(user_cb_data, [0, 1])
                return 0

            def mirror_failure(self, user_cb_data, msg, url):
                self.mirror_failure_cnt += 1
                self.test_instance.assertIn(user_cb_data, [0, 1])
                return 0

        PackageDownloadCallbacks.test_instance = self

        repo = self.add_repo_rpm("rpm-repo1")

        query = libdnf5.rpm.PackageQuery(self.base)
        query.filter_name(["one"])
        query.filter_arch(["noarch"])
        self.assertEqual(query.size(), 2)

        downloader = libdnf5.repo.PackageDownloader(self.base)

        cbs = PackageDownloadCallbacks()
        self.base.set_download_callbacks(
            libdnf5.repo.DownloadCallbacksUniquePtr(cbs))

        user_data = 2
        for package in query:
            downloader.add(package, user_data)
            user_data *= 5

        downloader.download()

        # forcefully deallocate the downloader, to check cbs is still valid
        downloader = None
        gc.collect()

        cbs.user_cb_data_container.sort()
        self.assertEqual(cbs.user_cb_data_container, [
                         "Package: one-0:1-1.noarch", "Package: one-0:2-1.noarch"])

        self.assertEqual(cbs.start_cnt, 2)
        self.assertGreaterEqual(cbs.progress_cnt, 2)
        self.assertEqual(cbs.mirror_failure_cnt, 0)
        self.assertEqual(cbs.end_cnt, 2)

        self.assertEqual(cbs.user_data_array, [2, 10])

        cbs.user_cb_data_array.sort()
        self.assertEqual(cbs.user_cb_data_array, [0, 1])
        self.assertEqual(
            cbs.end_status,
            [PackageDownloadCallbacks.TransferStatus_SUCCESSFUL, PackageDownloadCallbacks.TransferStatus_SUCCESSFUL])
        self.assertEqual(cbs.end_msg, [None, None])
