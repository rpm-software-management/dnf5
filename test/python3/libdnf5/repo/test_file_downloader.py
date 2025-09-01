# Copyright Contributors to the DNF5 project.
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

import os
import unittest

import libdnf5.base

import base_test_case


class TestFileDownloader(base_test_case.BaseTestCase):
    USER_DATA = 25

    def test_package_downloader(self):
        class DownloadCallbacks(libdnf5.repo.DownloadCallbacks):
            USER_CB_DATA = 5
            test_instance = None

            def __init__(self):
                super(DownloadCallbacks, self).__init__()
                self.start_cnt = 0
                self.progress_cnt = 0
                self.mirror_failure_cnt = 0
                self.end_cnt = 0
                self.end_status = None
                self.end_msg = None

            def add_new_download(self, user_data, description, total_to_download):
                self.start_cnt += 1
                self.test_instance.assertEqual(
                    user_data, self.test_instance.USER_DATA)
                return self.USER_CB_DATA

            def end(self, user_cb_data, status, msg):
                self.end_cnt += 1
                self.test_instance.assertEqual(user_cb_data, self.USER_CB_DATA)
                self.end_status = status
                self.end_msg = msg
                return 0

            def progress(self, user_cb_data, total_to_download, downloaded):
                self.progress_cnt += 1
                self.test_instance.assertEqual(user_cb_data, self.USER_CB_DATA)
                return 0

            def mirror_failure(self, user_cb_data, msg, url):
                self.mirror_failure_cnt += 1
                self.test_instance.assertEqual(user_cb_data, self.USER_CB_DATA)
                return 0

        DownloadCallbacks.test_instance = self

        source_file_path = os.path.join(
            base_test_case.PROJECT_SOURCE_DIR, "test/data/keys/key.pub")
        source_url = "file://" + source_file_path
        dest_file_path = os.path.join(self.temp_dir, "file_downloader.pub")

        cbs = DownloadCallbacks()
        self.base.set_download_callbacks(
            libdnf5.repo.DownloadCallbacksUniquePtr(cbs))

        file_downloader = libdnf5.repo.FileDownloader(self.base)
        file_downloader.add(source_url, dest_file_path, self.USER_DATA)
        file_downloader.download()

        self.assertEqual(cbs.start_cnt, 1)
        self.assertGreaterEqual(cbs.progress_cnt, 1)
        self.assertEqual(cbs.mirror_failure_cnt, 0)
        self.assertEqual(cbs.end_cnt, 1)

        self.assertEqual(
            cbs.end_status, DownloadCallbacks.TransferStatus_SUCCESSFUL)
        self.assertEqual(cbs.end_msg, None)
