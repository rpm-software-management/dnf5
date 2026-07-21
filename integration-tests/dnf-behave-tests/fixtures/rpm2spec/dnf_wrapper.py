import os
import shutil
import tempfile

import dnf


class DnfBaseWrapper(object):
    def __init__(self, arch=None, cache_dir=None):
        # prepopulate dict, everything else is redirected to the base object
        self.__dict__["tmp_dir"] = None
        self.__dict__["base"] = None
        self.__dict__["conf"] = None

        self.tmp_dir = tempfile.mkdtemp(prefix="dnf_")

        self.conf = dnf.conf.Conf()

        if cache_dir:
            self.conf.cachedir = cache_dir
        else:
            self.conf.cachedir = os.path.join(self.tmp_dir, "cache")

        if arch:
            # override runtime arch if requested
            self.conf.substitutions["arch"] = arch
            self.conf.substitutions["basearch"] = dnf.rpm.basearch(self.conf.substitutions["arch"])

        self.conf.assumeyes = False
        self.base = dnf.Base(conf=self.conf)

    def __del__(self):
        try:
            if self.tmp_dir:
                shutil.rmtree(self.tmp_dir)
        except:
            pass

    def __getattr__(self, name):
        return getattr(self.base, name)

    def __setattr__(self, name, value):
        if name in self.__dict__:
            self.__dict__[name] = value
            return
        setattr(self.base, name, value)

    def add_repo(self, repo_id, baseurl):
        if baseurl.startswith("/"):
            baseurl = "file://" + baseurl
        return self.base.repos.add_new_repo(repo_id, self.base.conf, baseurl=[baseurl], skip_if_unavailable=False)

    def fill_sack(self):
        self.base.fill_sack(load_system_repo=False, load_available_repos=True)
