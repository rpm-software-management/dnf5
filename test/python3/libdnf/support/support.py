import os
import shutil
import tempfile
import unittest

import libdnf


PROJECT_BINARY_DIR = os.environ["PROJECT_BINARY_DIR"]
PROJECT_SOURCE_DIR = os.environ["PROJECT_SOURCE_DIR"]


class LibdnfTestCase(unittest.TestCase):
    def setUp(self):
        self.base = libdnf.base.Base()

        self.cachedir = tempfile.mkdtemp(prefix="libdnf-test-python3-")
        self.base.get_config().cachedir().set(libdnf.conf.Option.Priority_RUNTIME, self.cachedir)

        self.repo_sack = self.base.get_repo_sack()
        self.sack = self.base.get_rpm_package_sack()

    def tearDown(self):
        shutil.rmtree(self.cachedir)

    """
    Add (load) a repo from `repo_path`.
    It's also a shared code for add_repo_repomd() and add_repo_rpm().
    """
    def _add_repo(self, repoid, repo_path):
        repo = self.repo_sack.new_repo(repoid)

        # set the repo baseurl
        repo.get_config().baseurl().set(libdnf.conf.Option.Priority_RUNTIME, "file://" + repo_path)

        # load repository into rpm.Repo
        repo.load()

        # load repo content into rpm.PackageSack
        self.sack.load_repo(repo.get(),
            libdnf.rpm.PackageSack.LoadRepoFlags_USE_FILELISTS |
            libdnf.rpm.PackageSack.LoadRepoFlags_USE_OTHER |
            libdnf.rpm.PackageSack.LoadRepoFlags_USE_PRESTO |
            libdnf.rpm.PackageSack.LoadRepoFlags_USE_UPDATEINFO
        )

    """
    Add (load) a repo from PROJECT_SOURCE_DIR/test/data/repos-repomd/<repoid>/repodata
    """
    def add_repo_repomd(self, repoid):
        repo_path = os.path.join(PROJECT_SOURCE_DIR, "test/data/repos-repomd", repoid)
        self._add_repo(repoid, repo_path)

    """
    Add (load) a repo from PROJECT_BINARY_DIR/test/data/repos-rpm/<repoid>/repodata
    """
    def add_repo_rpm(self, repoid):
        repo_path = os.path.join(PROJECT_BINARY_DIR, "test/data/repos-rpm", repoid)
        self._add_repo(repoid, repo_path)


    """
    Add (load) a repo from PROJECT_SOURCE_DIR/test/data/repos-solv/<repoid>.repo
    """
    def add_repo_solv(self, repoid):
        repo_path = os.path.join(PROJECT_SOURCE_DIR, "test/data/repos-solv", repoid + ".repo")
        self.repo_sack.new_repo_from_libsolv_testcase(repoid, repo_path)
