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
    def test_load_system_repo(self):
        # TODO(lukash) there's no rpmdb in the installroot, create data for the test
        self.repo_sack.get_system_repo().load()

    def test_load_repo(self):
        repoid = "repomd-repo1"
        repo = self.add_repo_repomd(repoid, load=False)

        class DownloadCallbacks(libdnf5.repo.DownloadCallbacks):
            start_cnt = 0
            start_what = None

            end_cnt = 0
            end_error_message = None

            progress_cnt = 0
            fastest_mirror_cnt = 0
            handle_mirror_failure_cnt = 0

            def add_new_download(self, user_data, descr, total):
                self.start_cnt += 1
                self.start_what = descr
                return None

            def end(self, user_cb_data, status, error_message):
                self.end_cnt += 1
                self.end_error_message = error_message
                return 0

            def progress(self, user_cb_data, total_to_download, downloaded):
                self.progress_cnt += 1
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

        dl_cbs = DownloadCallbacks()
        self.base.set_download_callbacks(
            libdnf5.repo.DownloadCallbacksUniquePtr(dl_cbs))

        cbs = RepoCallbacks()
        # TODO(lukash) try to wrap the creation of the unique_ptr so that cbs
        # can be passed directly to repo.set_callbacks
        repo.set_callbacks(libdnf5.repo.RepoCallbacksUniquePtr(cbs))

        repos = libdnf5.repo.RepoQuery(self.base)
        repos.filter_id(repoid)
        self.repo_sack.update_and_load_repos(repos)

        self.assertEqual(dl_cbs.start_cnt, 1)
        self.assertEqual(dl_cbs.start_what, repoid)

        self.assertEqual(dl_cbs.end_cnt, 1)
        self.assertEqual(dl_cbs.end_error_message, None)

        self.assertGreaterEqual(dl_cbs.progress_cnt, 1)
        self.assertEqual(dl_cbs.fastest_mirror_cnt, 0)
        self.assertEqual(dl_cbs.handle_mirror_failure_cnt, 0)
        self.assertEqual(cbs.repokey_import_cnt, 0)

    def test_load_extra_system_repo_incorrectly(self):
        # Try to load extra system repo on non-system repo
        repo = self.repo_sack.create_repo('test')
        self.assertRaises(
            RuntimeError, repo.load_extra_system_repo, 'some-root-dir')

        # Try to load extra system repo on non-loaded repo
        repo = self.repo_sack.get_system_repo()
        self.assertRaises(RuntimeError, repo.load_extra_system_repo, 'dir')
