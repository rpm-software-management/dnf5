# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave
import os
import tomllib

from common.lib.behave_ext import check_context_table
from common.lib.diff import print_lines_diff
from lib.rpm import RPM


@behave.then('package state is')
def package_state_is(context):
    """
    Checks the reason in system state packages.toml as well as the from_repo
    attribute from nevras.toml in one step. For that, the table in context has
    to contain the full NEVRA, which is converted to NA for checking the
    reason.

    The reason column also supports value of "None" which represents no
    record for the given NA.

    For installonly packages, multiple NEVRAS for the same NA can be put into
    the table, the reason is checked just as one NA record in packages.toml.
    """
    check_context_table(context, ["package", "reason", "from_repo"])

    found_pkgs = []
    with open(os.path.join(context.dnf.installroot, "usr/lib/sysimage/libdnf5/packages.toml"), "rb") as f:
        for k, v in tomllib.load(f)["packages"].items():
            found_pkgs.append((k, v["reason"]))
    found_pkgs.sort()

    found_nevras = []
    with open(os.path.join(context.dnf.installroot, "usr/lib/sysimage/libdnf5/nevras.toml"), "rb") as f:
        for k, v in tomllib.load(f)["nevras"].items():
            found_nevras.append((k, v["from_repo"]))
    found_nevras.sort()

    expected_pkgs_dict = {}
    expected_nevras = []
    for package, reason, from_repo in context.table:
        if reason != "None":
            na = RPM(package).na
            if na in expected_pkgs_dict and expected_pkgs_dict[na] != reason:
                raise AssertionError("Inconsistent reason for NA \"{}\"".format(na))
            expected_pkgs_dict[na] = reason

        expected_nevras.append((package, from_repo))

    expected_pkgs = sorted(expected_pkgs_dict.items())
    expected_nevras.sort()

    fail = False
    if expected_pkgs != found_pkgs:
        print("packages.toml system state differs from expected:")
        print_lines_diff(expected_pkgs, found_pkgs)
        fail = True

    if expected_nevras != found_nevras:
        print("nevras.toml system state differs from expected:")
        print_lines_diff(expected_nevras, found_nevras)
        fail = True

    if fail:
        raise AssertionError("System state mismatch")


@behave.then('group state is')
def group_state_is(context):
    """
    Checks packages and userinstalled state in groups system state groups.toml.
    For that, the table in context has to contain group id.
    """
    check_context_table(context, ["id", "package_types", "packages", "userinstalled"])

    found_groups = []
    with open(os.path.join(context.dnf.installroot, "usr/lib/sysimage/libdnf5/groups.toml"), "rb") as f:
        for k, v in tomllib.load(f)["groups"].items():
            pkg_types = v["package_types"]
            pkg_types.sort()
            pkgs = v["packages"]
            pkgs.sort()
            found_groups.append((k, ', '.join(pkg_types), ', '.join(pkgs), str(v["userinstalled"])))
    found_groups.sort()

    expected_groups = []
    for group_id, package_types, packages, userinstalled in context.table:
        p_types = package_types.split(',')
        p_types = list(map(str.strip, p_types))
        p_types.sort()
        p = packages.split(',')
        p = list(map(str.strip, p))
        p.sort()
        expected_groups.append((group_id, ', '.join(p_types), ', '.join(p), str(userinstalled)))

    expected_groups.sort()

    fail = False
    if expected_groups != found_groups:
        print("groups.toml system state differs from expected:")
        print_lines_diff(expected_groups, found_groups)
        fail = True

    if fail:
        raise AssertionError("Group system state mismatch")


@behave.then('environment state is')
def environment_state_is(context):
    """
    Checks groups in environments system state environments.toml.
    For that, the table in context has to contain environment id.
    """
    check_context_table(context, ["id", "groups"])

    found_environments = []
    with open(os.path.join(context.dnf.installroot, "usr/lib/sysimage/libdnf5/environments.toml"), "rb") as f:
        for k, v in tomllib.load(f)["environments"].items():
            groups = v["groups"]
            groups.sort()
            found_environments.append((k, ', '.join(groups)))
    found_environments.sort()

    expected_environments = []
    for env_id, groups in context.table:
        g = groups.split(',')
        g = list(map(str.strip, g))
        g.sort()
        expected_environments.append((env_id, ', '.join(g)))

    expected_environments.sort()

    fail = False
    if expected_environments != found_environments:
        print("environments.toml system state differs from expected:")
        print_lines_diff(expected_environments, found_environments)
        fail = True

    if fail:
        raise AssertionError("Environment system state mismatch")
