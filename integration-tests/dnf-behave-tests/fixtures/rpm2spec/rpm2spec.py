#!/usr/bin/python3


import errno
import os

from dnf_wrapper import DnfBaseWrapper


REPOS = [
    "fedora",
    "fedora-debuginfo",
    "fedora-source",
    "updates",
    "updates-debuginfo",
    "updates-source",
    "updates-testing",
    "updates-testing-debuginfo",
    "updates-testing-source",
    "fedora-modular",
    "fedora-modular-debuginfo",
    "fedora-modular-source",
    "updates-modular",
    "updates-modular-debuginfo",
    "updates-modular-source",
]


PACKAGES = [
    # source repo, package, target repo

    # dnf-ci-fedora
    ["fedora", "setup", "dnf-ci-fedora"],
    ["fedora", "filesystem", "dnf-ci-fedora"],
    ["fedora", "basesystem", "dnf-ci-fedora"],
    ["fedora", "glibc", "dnf-ci-fedora"],
    ["fedora", "ninja-build", "dnf-ci-fedora"],

    # dnf-ci-fedora-updates
    ["updates", "glibc", "dnf-ci-fedora-updates"],
    ["updates", "zchunk", "dnf-ci-fedora-updates"],
    ["fedora", "libzstd", "dnf-ci-fedora-updates"],

    # dnf-ci-fedora-modular
    #    ["fedora-modular", "nodejs", "dnf-ci-fedora-modular"],
    #    ["fedora-modular", "postgresql", "dnf-ci-fedora-modular"],

    # dnf-ci-fedora-modular-updates
    ["updates-modular", "nodejs", "dnf-ci-fedora-modular-updates"],
    ["updates-modular", "postgresql", "dnf-ci-fedora-modular-updates"],
]

cache_dir = os.path.abspath("_cache")
try:
    os.makedirs(cache_dir)
except OSError as ex:
    if ex.errno != errno.EEXIST:
        raise


d = DnfBaseWrapper(arch="x86_64", cache_dir=cache_dir)


for repo in REPOS:
    baseurl = "file://" + os.path.join(os.path.abspath(os.path.dirname(__file__)), "repos", repo)
    d.add_repo(repo, baseurl)

d.fill_sack()


class Package(object):
    def __init__(self, pkg, main_package):
        self._pkg = pkg
        self._main_package = main_package

    def __getattr__(self, name):
        return getattr(self._pkg, name)

    def _package_name(self):
        if self.name.startswith(self._main_package.name + "-"):
            return self.name[len(self._main_package.name) + 1:]
        if self.name == self._main_package.name:
            return ""
        return "-n {}".format(self.name)

    def package(self):
        result = []
        if self.name != self._main_package.name:
            result.append("%package {}".format(self._package_name()).strip())
        result.append("Summary:        {}".format(self.summary))
        if self.arch == "noarch":
            result.append("BuildArch:      {}".format("noarch"))

        fields = ["Provides", "Requires", "Conflicts", "Obsoletes"]
        fields += ["Recommends", "Supplements", "Suggests", "Enhances"]
        for field in fields:
            values = getattr(self, field.lower())
            if not values:
                continue
            result.append("")
            for i in values:
                result.append("{:16}{}".format(field + ":", i))

        result.append("")
        result.append("%description {}".format(self._package_name()).strip())
        result.append(self.description or "")

        return result

    def files(self):
        result = []
        result.append("")
        result.append("%files {}".format(self._package_name()).strip())

        # HACK: don't return any files, we don't want them in the test spec files
        return result

        all_dirs = self._main_package.all_dirs
        for i in self._pkg.files:
            if i in all_dirs:
                result.append("%dir   {}".format(i))
            else:
                result.append("%ghost {}".format(i))
        return result


class Spec(object):
    def __init__(self, pkg):
        self._pkg = pkg
        self._packages = {}

    def __getattr__(self, name):
        result = getattr(self._pkg, name)
        return result

    @property
    def is_noarch(self):
        for name, package in sorted(self._packages.items()):
            if package.arch != "noarch":
                return False
        return True

    @property
    def has_debuginfo(self):
        for name, package in sorted(self._packages.items()):
            if "debuginfo" in package.name:
                return True
        return False

    @property
    def all_dirs(self):
        files = set()
        for name, package in sorted(self._packages.items()):
            files.update(package._pkg.files)
        dirs = set()
        for i in files:
            dirname = os.path.dirname(i)
            if dirname == "/":
                continue
            if dirname in files:
                dirs.add(dirname)
        return dirs

    def header(self):
        result = []
        if self.has_debuginfo:
            result.append("%undefine _debuginfo_subpackages")
            result.append("")

        result.append("Name:           {}".format(self.name))
        result.append("Epoch:          {}".format(self.epoch or 0))
        result.append("Version:        {}".format(self.version))
        result.append("Release:        {}".format(self.release))
        result.append("")
        result.append("License:        {}".format(self.license))
        result.append("URL:            {}".format(self.url))
        return result

    def __str__(self):
        result = []
        result.extend(self.header())

        # make sure the main package goes first
        pkgs = self._packages.copy()
        if self.name in pkgs:
            pkg = pkgs.pop(self.name)
            pkgs_list = sorted(pkgs.items())
            pkgs_list.insert(0, (self.name, pkg))
        else:
            pkgs_list = sorted(pkgs.items())

        for name, package in pkgs_list:
            result.append("")
            result.extend(package.package())
        for name, package in sorted(self._packages.items()):
            result.extend(package.files())
        result.append("")
        result.append("%changelog")
        result.append("")
        return "\n".join(result)

    def add_package(self, pkg):
        self._packages[pkg.name] = Package(pkg, self)


for repo, package, target_repo in PACKAGES:
    # TODO: latest doesn't work
    # q = d.sack.query().filter(latest=True, name=PKG, reponame=REPO)
    q = d.sack.query().filter(name=package, reponame=repo)

    # TODO: if q is empty -> SIGSEGV
    srpm = q[0].sourcerpm

    q = d.sack.query().filter(sourcerpm=srpm)

    src_q = d.sack.query().filter(nevra=srpm.replace(".rpm", ""))
    # BUG: q[0] -> segfault if there's no match

    spec_dir = os.path.join("specs", target_repo)
    try:
        os.makedirs(spec_dir)
    except OSError as ex:
        if ex.errno != errno.EEXIST:
            raise

    for srpm_pkg in src_q:
        spec = Spec(srpm_pkg)

        for pkg in q:
            spec.add_package(pkg)

        spec_file = os.path.join(spec_dir, "%s-%s-%s.spec" % (spec.name, spec.version, spec.release))
        open(spec_file, "w").write(str(spec))
