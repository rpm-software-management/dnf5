# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import re

import rpm


NEVRA_RE = re.compile(r"^(.+)-(?:([0-9]+):)?(.+)-(.+)\.(.+)$")


class RPM(object):
    @staticmethod
    def parse(nevra):
        match = NEVRA_RE.match(nevra)
        if not match:
            raise ValueError("Cannot parse NEVRA: %s" % nevra)
        result = list(match.groups())
        result[1] = int(result[1]) if result[1] is not None else 0
        return result

    def __init__(self, nevra, rpmheader=None):
        nevra_split = self.parse(nevra)
        self.name = nevra_split[0]
        self.epoch = nevra_split[1]
        self.version = nevra_split[2]
        self.release = nevra_split[3]
        self.arch = nevra_split[4]
        self.rpmheader = rpmheader

    def __str__(self):
        return "%s-%d:%s-%s.%s" % (self.name, self.epoch, self.version, self.release, self.arch)

    def __repr__(self):
        return "<%s: %s>" % (self.__class__.__name__, self)

    def __hash__(self):
        return hash(str(self))

    def __eq__(self, other):
        if self.name != other.name:
            return False
        if self.epoch != other.epoch:
            return False
        if self.version != other.version:
            return False
        if self.release != other.release:
            return False
        if self.arch != other.arch:
            return False
        return True

    def __lt__(self, other):
        if (self.na < other.na):
            return True
        if (self.na > other.na):
            return False
        one = (str(self.epoch), self.version, self.release)
        two = (str(other.epoch), other.version, other.release)
        return rpm.labelCompare(one, two) <= -1

    @property
    def na(self):
        return "%s.%s" % (self.name, self.arch)


def normalize_epoch(evr):
    if ":" not in evr:
        # prepend "0:" if there's no epoch specified
        return "0:" + evr
    return evr


def diff_rpm_lists(list_one, list_two):
    result = {
        # actions
        "install": [],
        "remove": [],
        "upgrade": [],
        "upgraded": [],
        "downgrade": [],
        "downgraded": [],

        # it is not clear whether a RPM was reinstalled or not changed at all
        # use "unchanged" instead
        "reinstall": [],
        "obsoleted": [],

        "changed": [],
        "unchanged": [],
        "present": [],
    }

    list_one = sorted(list_one)
    list_two = sorted(list_two)

    # automaticaly detect names of installonly packages
    # ASSUMPTION: multiple versions of a package with the same name.arch on the system means installonly the package
    installonly_names = set()
    prev_pkg = None
    for pkg in list_one:
        if prev_pkg and pkg.na == prev_pkg.na and pkg != prev_pkg:
            installonly_names.add(pkg.na)
        prev_pkg = pkg
    prev_pkg = None
    for pkg in list_two:
        if prev_pkg and pkg.na == prev_pkg.na and pkg != prev_pkg:
            installonly_names.add(pkg.na)
        prev_pkg = pkg

    # PRESENT PACKAGES AND UNCHANGED INSTALLONLY PACKAGES
    for pkg in list_two:
        result["present"].append(pkg)
        if (pkg in list_one) and (pkg.na in installonly_names):
            result["unchanged"].append(pkg)
    for pkg in result["unchanged"]:
        list_one.remove(pkg)
        list_two.remove(pkg)

    names_one = set([i.na for i in list_one if i.na not in installonly_names])
    names_two = set([i.na for i in list_two if i.na not in installonly_names])

    # INSTALL
    to_install = []
    for pkg in list_two:
        # installonly pkgs get always installed
        if pkg.na in installonly_names:
            to_install.append(pkg)
            continue
        # detect upgrades/downgrades etc.
        if pkg.na in names_one:
            continue
        to_install.append(pkg)
    for pkg in to_install:
        result["install"].append(pkg)
        list_two.remove(pkg)

    # REMOVE
    to_remove = []
    for pkg in list_one:
        # installonly pkgs get always removed
        if pkg.na in installonly_names:
            to_remove.append(pkg)
            continue
        # detect upgrades/downgrades etc.
        if pkg.na in names_two:
            continue
        to_remove.append(pkg)
    for pkg in to_remove:
        result["remove"].append(pkg)
        list_one.remove(pkg)

    names_one = set([i.na for i in list_one if i.na not in installonly_names])
    names_two = set([i.na for i in list_two if i.na not in installonly_names])
    assert names_one == names_two

    # UNCHANGED / REINSTALLED
    unchanged_reinstalled = set()
    for pkg_two in list_two:
        if pkg_two in list_one:
            unchanged_reinstalled.add(pkg_two)

    # remove all 'unchanged' packages from both lists
    list_one = [i for i in list_one if i not in unchanged_reinstalled]
    list_two = [i for i in list_two if i not in unchanged_reinstalled]
    unchanged_names = set([i.na for i in unchanged_reinstalled])
    reinstall_names = set()

    # ASSUMPTION: An 'unchanged' package is 'reinstall'
    # if there's another action for the same package name.
    # This happens mainly if rpmdb is broken, containing duplicates
    # and dnf enforces reinstall in these cases.

    for name in unchanged_names.copy():
        for pkg_one in list_one:
            if pkg_one.na != name:
                continue
            result["obsoleted"].append(pkg_one)
            list_one.remove(pkg_one)
            if name in unchanged_names:
                unchanged_names.remove(name)
                reinstall_names.add(name)

    for pkg in unchanged_reinstalled:
        if pkg.na in unchanged_names:
            result["unchanged"].append(pkg)
        else:
            result["reinstall"].append(pkg)

    for pkg_one in list_one:
        if pkg_one.na in unchanged_names:
            reinstall_names.add(pkg_one.na)
            unchanged_names.add(pkg_one.na)
            result["reinstall"].append(pkg_one)
        else:
            result["unchanged"].append(pkg_one)

    for pkg_one in list_one:
        for pkg_two in list_two:
            if pkg_one.na != pkg_two.na:
                continue
            if pkg_one < pkg_two:
                result["upgraded"].append(pkg_one)
                result["upgrade"].append(pkg_two)
            elif pkg_one > pkg_two:
                result["downgraded"].append(pkg_one)
                result["downgrade"].append(pkg_two)
            else:
                result["unchanged"].append(pkg_two)

    # construct 'changed' list which is used to detect if transaction was empty
    # 'remove' packages will not be 'present' so add them
    result["changed"] = result["present"] + result["remove"]
    for pkg in result["unchanged"]:
        if pkg in result["changed"]:
            result["changed"].remove(pkg)

    return result
