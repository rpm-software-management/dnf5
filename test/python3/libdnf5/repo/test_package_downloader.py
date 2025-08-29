# Copyright Contributors to the DNF5 project.
# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

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


class TestPackageDownloaderReturnNone(base_test_case.BaseTestCase):
    # Previously, add_new_download could only return None. This test adds a
    # check to ensure backwards-compatibility with that API.
    def test_package_downloader_return_none(self):
        class PackageDownloadCallbacks(libdnf5.repo.DownloadCallbacks):
            def __init__(self):
                super(PackageDownloadCallbacks, self).__init__()

            def add_new_download(self, user_data, description, total_to_download):
                return None

        repo = self.add_repo_rpm("rpm-repo1")

        query = libdnf5.rpm.PackageQuery(self.base)
        query.filter_name(["one"])
        query.filter_arch(["noarch"])
        self.assertEqual(query.size(), 2)

        downloader = libdnf5.repo.PackageDownloader(self.base)

        cbs = PackageDownloadCallbacks()
        self.base.set_download_callbacks(
            libdnf5.repo.DownloadCallbacksUniquePtr(cbs))

        for package in query:
            downloader.add(package)

        downloader.download()
