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

import libdnf5.base

import base_test_case


class TestRepo(base_test_case.BaseTestCase):
    class DownloadCallbacks(libdnf5.repo.DownloadCallbacks):
        end_cnt = 0
        end_error_message = None

        fastest_mirror_cnt = 0
        handle_mirror_failure_cnt = 0

        def end(self, user_cb_data, status, error_message):
            self.end_cnt += 1
            self.end_error_message = error_message
            return 0

        def fastest_mirror(self, user_cb_data, stage, ptr):
            self.fastest_mirror_cnt += 1

        def handle_mirror_failure(self, user_cb_data, msg, url, metadata):
            self.handle_mirror_failure_cnt += 1
            return 0

    class RepoCallbacks(libdnf5.repo.RepoCallbacks):
        repokey_import_cnt = 0

        def repokey_import(self, id, user_id, fingerprint, url, timestamp):
            self.repokey_import_cnt += 1
            return True

    def test_load_repo(self):
        repoid = "repomd-repo1"
        repo = self.add_repo_repomd(repoid, load=False)

        dl_cbs = self.DownloadCallbacks()
        self.base.set_download_callbacks(
            libdnf5.repo.DownloadCallbacksUniquePtr(dl_cbs))

        cbs = self.RepoCallbacks()
        # TODO(lukash) try to wrap the creation of the unique_ptr so that cbs
        # can be passed directly to repo.set_callbacks
        repo.set_callbacks(libdnf5.repo.RepoCallbacksUniquePtr(cbs))

        self.repo_sack.load_repos(libdnf5.repo.Repo.Type_AVAILABLE)

        self.assertEqual(dl_cbs.end_cnt, 1)
        self.assertEqual(dl_cbs.end_error_message, None)

        self.assertEqual(dl_cbs.fastest_mirror_cnt, 0)
        self.assertEqual(dl_cbs.handle_mirror_failure_cnt, 0)
        self.assertEqual(cbs.repokey_import_cnt, 0)

    def test_load_repo_overload(self):
        repoid = "repomd-repo1"
        repo = self.add_repo_repomd(repoid, load=False)

        dl_cbs = self.DownloadCallbacks()
        self.base.set_download_callbacks(
            libdnf5.repo.DownloadCallbacksUniquePtr(dl_cbs))

        cbs = self.RepoCallbacks()
        repo.set_callbacks(libdnf5.repo.RepoCallbacksUniquePtr(cbs))

        self.repo_sack.load_repos()

        self.assertEqual(dl_cbs.end_cnt, 1)
        self.assertEqual(dl_cbs.end_error_message, None)

        self.assertEqual(dl_cbs.fastest_mirror_cnt, 0)
        self.assertEqual(dl_cbs.handle_mirror_failure_cnt, 0)
        self.assertEqual(cbs.repokey_import_cnt, 0)
