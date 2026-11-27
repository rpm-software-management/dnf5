# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave
import dbus
import os
import re

from cmd import execute_python_script
from common.lib.behave_ext import check_context_table
from common.lib.diff import print_lines_diff
from lib.dbus_session import (
    DbusSession,
    convert_table_value,
    dbus_to_python,
)

# make the helpers from the steps lib (e.g. dbus_to_python) importable. It is
# formatted here at import time - it must not contain any braces afterwards,
# because the whole script is .format(context=context)-ed later.
_LOCAL_IMPORT_PATH = """
import sys
sys.path.insert(0, "{0}")

""".format(os.path.dirname(os.path.abspath(__file__)))

DBUS_SESSION_SETUP = _LOCAL_IMPORT_PATH + """
import dbus

from lib.dbus_session import (
    DNFDAEMON_BUS_NAME,
    DNFDAEMON_OBJECT_PATH,
    IFACE_SESSION_MANAGER,
    IFACE_REPO,
    IFACE_REPOCONF,
    IFACE_RPM,
    IFACE_GOAL,
    IFACE_HISTORY,
    IFACE_ADVISORY,
    IFACE_GROUP,
    dbus_to_python,
)

bus = dbus.SystemBus()
iface_session = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, DNFDAEMON_OBJECT_PATH),
    dbus_interface=IFACE_SESSION_MANAGER)

session = iface_session.open_session({{
    "config": {{
        "installroot": "{context.dnf.installroot}",
        "optional_metadata_types": "updateinfo",
        }}
    }})
"""


@behave.step("I execute python libdnf5 dbus api script with session")
def execute_python_libdnf5_dbus_api_script_with_session(context):
    """
    Execute snippet of python script using libdnf5 dbus api that is
    appended to prepared D-Bus session.
    """
    execute_python_script(context, DBUS_SESSION_SETUP + context.text)


@behave.step("I execute python libdnf5 dbus api script with history interface")
def execute_python_libdnf5_dbus_api_script_with_history_interface(context):
    """
    Execute snippet of python script using libdnf5 dbus api that is
    appended to prepared D-Bus session.
    Helper functions to work with History interface are included.
    """
    history_helpers = """
def print_recent_history_pkgs(pkglist):
    for pkg in pkglist:
        print('NEVRA: {{pkg[name]}}-{{pkg[evr]}}.{{pkg[arch]}}'.format(pkg=pkg))
        if "summary" in pkg:
            print('Summary: {{pkg[summary]}}'.format(pkg=pkg))
        if "original_evr" in pkg:
            print("Original EVR: {{pkg[original_evr]}}".format(pkg=pkg))
        if "original_arch" in pkg:
            print("Original arch: {{pkg[original_arch]}}".format(pkg=pkg))
        if "transaction_time" in pkg:
            print("Transaction time: {{}}".format(int(pkg["transaction_time"])))
        if "advisories" in pkg:
            print("Advisories: {{}}".format(", ".join(pkg["advisories"])))

def print_recent_history(dbus_output):
    for key in ["installed", "upgraded", "downgraded", "removed"]:
        pkgs = dbus_output.get(key, [])
        print("{{}}: {{}}".format(key, len(pkgs)))
        print_recent_history_pkgs(pkgs)

iface_history = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, session),
    dbus_interface=IFACE_HISTORY)
"""
    execute_python_script(context, DBUS_SESSION_SETUP + history_helpers + context.text)


@behave.step("I execute python libdnf5 dbus api script with group interface")
def execute_python_libdnf5_dbus_api_script_with_group_interface(context):
    """
    Execute snippet of python script using libdnf5 dbus api that is
    appended to prepared D-Bus session.
    Helper functions to work with Group interface are included.
    """
    group_helpers = """
iface_group = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, session),
    dbus_interface=IFACE_GROUP)
"""
    execute_python_script(context, DBUS_SESSION_SETUP + group_helpers + context.text)


# --- Structured dnf5daemon D-Bus API testing ---
#
# Unlike the "execute python script" steps above, the following steps talk to
# the daemon directly from the behave process. A D-Bus session opened by a
# step stays available to all subsequent steps of the scenario (the daemon
# only allows session method calls for the D-Bus sender that opened it).

RPM_METHODS = ("install", "remove", "upgrade", "downgrade", "reinstall", "distro_sync")

TRANSACTION_TABLE_HEADINGS = [
    "action", "reason", "full_nevra", "repo_id", "from_repo_id", "package_reason",
]

# the package attributes compared by "the D-Bus transaction is"; everything else
# the daemon returns (name, epoch, evr, ..., id, download_size, install_size,
# ...) is ignored - the NEVRA components are covered by full_nevra
PKG_ATTRS = [
    "full_nevra", "repo_id", "from_repo_id", "reason",
]

# conversion of option values from behave table cells
DBUS_OPTION_PARSERS = {
    "skip_unavailable": lambda value: value == "True",
    "repo_ids": lambda value: [i.strip() for i in value.split(",") if i.strip()],
}


@behave.fixture
def dbus_session_fixture(context, session_config=None):
    """Open a dnf5daemon session for the scenario; close it when the
    scenario ends."""
    context.dbus_session = DbusSession(context, session_config)
    yield context.dbus_session
    session = context.dbus_session
    if not session.closed:
        try:
            session.close()
        except dbus.exceptions.DBusException:
            pass
    context.dbus_session = None


def assert_sorted_equal(expected, found, description):
    """Order-insensitive comparison of two lists of plain python values.

    Prints a diff of the sorted items before failing.
    """
    expected_sorted = sorted(expected, key=repr)
    found_sorted = sorted(found, key=repr)
    if expected_sorted != found_sorted:
        print_lines_diff([repr(i) for i in expected_sorted],
                         [repr(i) for i in found_sorted])
        raise AssertionError("%s does not match" % description)


@behave.given("I open a dnf5daemon session")
def open_dnf5daemon_session(context):
    """Open a dnf5daemon session configured for the current installroot."""
    behave.use_fixture(dbus_session_fixture, context)


@behave.step("I close the dnf5daemon session")
def close_dnf5daemon_session(context):
    """Close the dnf5daemon session."""
    session = context.dbus_session
    session.close_result = session.session_manager.close_session(session.object_path)
    session.closed = True


@behave.step("the dnf5daemon session close returns {result}")
def dnf5daemon_session_close_returns(context, result):
    assert context.dbus_session.close_result == (result == "True"), \
        "Unexpected close_session return value: %r" % (context.dbus_session.close_result,)


@behave.step("the dnf5daemon session object path matches \"{regex}\"")
def dnf5daemon_session_object_path_matches(context, regex):
    object_path = context.dbus_session.object_path
    assert re.match(regex, object_path), \
        "Session object path %r does not match %r" % (object_path, regex)


def dbus_rpm_action(context, action, specs, options=None):
    method = action.replace('-', '_')
    assert method in RPM_METHODS, "Unknown rpm action: %s" % action
    session = context.dbus_session
    session.rpm_action(method, specs, options)


@behave.step("I {action} packages \"{specs}\" via D-Bus with options")
def dbus_rpm_action_with_options(context, action, specs):
    check_context_table(context, ["key", "value"])
    options = {}
    for row in context.table:
        key = row["key"]
        value = row["value"].format(context=context)
        options[key] = DBUS_OPTION_PARSERS[key](value) if key in DBUS_OPTION_PARSERS else value
    dbus_rpm_action(context, action, [spec.strip() for spec in specs.format(context=context).split(",")], options)


@behave.step("I {action} packages \"{specs}\" via D-Bus")
def dbus_rpm_action_plain(context, action, specs):
    dbus_rpm_action(context, action, [spec.strip() for spec in specs.format(context=context).split(",")])


@behave.step("I resolve the D-Bus goal")
def resolve_dbus_goal(context):
    context.dbus_session.resolve()


@behave.step("the D-Bus goal resolve result is {result:d}")
def dbus_goal_resolve_result_is(context, result):
    assert context.dbus_session.result == result, \
        "Unexpected goal resolve result: %r" % (context.dbus_session.result,)


@behave.step("I reset the D-Bus goal")
def reset_dbus_goal(context):
    context.dbus_session.goal.reset(dbus.Dictionary({}, signature='sv'))


@behave.step("I run the D-Bus transaction")
def run_dbus_transaction(context):
    context.dbus_session.goal.do_transaction(dbus.Dictionary({}, signature='sv'))


@behave.step("I attempt to run the D-Bus transaction")
def attempt_dbus_transaction(context):
    session = context.dbus_session
    session.error = None
    try:
        session.goal.do_transaction(dbus.Dictionary({}, signature='sv'))
    except dbus.exceptions.DBusException as error:
        session.error = error


@behave.step("a D-Bus error \"{name}\" is raised with message \"{message}\"")
def dbus_error_is_raised(context, name, message):
    error = context.dbus_session.error
    assert error is not None, "No D-Bus error was raised"
    assert error.get_dbus_name() == name, \
        "Unexpected D-Bus error name: %r" % (error.get_dbus_name(),)
    assert str(error) == "%s: %s" % (name, message), \
        "Unexpected D-Bus error message: %r" % (str(error),)


@behave.step("the D-Bus transaction is")
def dbus_transaction_is(context):
    """The table lists one row per expected transaction object. Only the listed
    attributes are compared - other attributes the daemon returns (package id,
    download_size, install_size, transaction item attributes, ...) are ignored."""
    check_context_table(context, TRANSACTION_TABLE_HEADINGS)
    expected = []
    for row in context.table:
        pkg = {
            "full_nevra": row["full_nevra"],
            "repo_id": row["repo_id"],
            "from_repo_id": row["from_repo_id"],
            "reason": row["package_reason"],
        }
        expected.append(("Package", row["action"], row["reason"], {}, pkg))
    found = []
    for object_type, action, reason, trans_item_attrs, pkg in dbus_to_python(
            context.dbus_session.resolved):
        assert object_type == "Package", \
            "Unexpected transaction object type: %r" % (object_type,)
        found.append(("Package", action, reason, {},
                      {attr: pkg.get(attr) for attr in PKG_ATTRS}))
    assert_sorted_equal(expected, found, "Resolved transaction")


@behave.step("the D-Bus transaction is empty")
def dbus_transaction_is_empty(context):
    assert dbus_to_python(context.dbus_session.resolved) == [], \
        "Unexpected transaction content: %r" % (context.dbus_session.resolved,)


@behave.step("the D-Bus transaction problems are")
def dbus_transaction_problems_are(context):
    assert context.text
    expected = [line.strip() for line in context.text.strip().splitlines()]
    found = dbus_to_python(context.dbus_session.goal.get_transaction_problems_string())
    assert_sorted_equal(expected, found, "Transaction problems")


@behave.step("I list packages via D-Bus matching \"{patterns}\" with attributes \"{attrs}\"")
def list_packages_dbus_matching(context, patterns, attrs):
    context.dbus_session.package_list = dbus_list_packages(
        context, attrs=attrs, patterns=patterns)


@behave.step("I list packages via D-Bus with attributes \"{attrs}\"")
def list_packages_dbus(context, attrs):
    context.dbus_session.package_list = dbus_list_packages(context, attrs=attrs)


def dbus_list_packages(context, attrs=None, patterns=None):
    options = {"package_attrs": attrs.split()}
    if patterns is not None:
        options["patterns"] = patterns.format(context=context).split()
    pkglist = context.dbus_session.rpm.list(dbus.Dictionary(options, signature='sv'))
    pkglist = dbus_to_python(pkglist)
    for pkg in pkglist:
        # the id of a package depends on the order of the repos in the sack
        # which varies between runs so we can't rely on the value
        pkg.pop('id', None)
    return pkglist


@behave.step("the D-Bus package list is")
def dbus_package_list_is(context):
    assert context.table
    attrs = list(context.table.headings)
    found = context.dbus_session.package_list
    # determine the value type of each attribute from the found data
    templates = {}
    for pkg in found:
        for attr in attrs:
            if attr in pkg and attr not in templates:
                templates[attr] = pkg[attr]
    expected = [
        {attr: convert_table_value(row[attr], templates.get(attr, "")) for attr in attrs}
        for row in context.table
    ]
    assert_sorted_equal(expected, found, "Package list")


@behave.step("I list repositories via D-Bus matching \"{pattern}\"")
def list_repositories_dbus_matching(context, pattern):
    context.dbus_session.repository_list = dbus_list_repositories(
        context, patterns=[pattern.format(context=context)])


@behave.step("I list repositories via D-Bus")
def list_repositories_dbus(context):
    context.dbus_session.repository_list = dbus_list_repositories(context)


def dbus_list_repositories(context, patterns=None):
    options = {"repo_attrs": ["name", "enabled"]}
    if patterns is not None:
        options["patterns"] = patterns
    return dbus_to_python(context.dbus_session.repo.list(
        dbus.Dictionary(options, signature='sv')))


@behave.step("the D-Bus repository list is")
def dbus_repository_list_is(context):
    assert context.table
    attrs = list(context.table.headings)
    found = context.dbus_session.repository_list
    templates = {}
    for repo in found:
        for attr in attrs:
            if attr in repo and attr not in templates:
                templates[attr] = repo[attr]
    expected = [
        {attr: convert_table_value(row[attr], templates.get(attr, "")) for attr in attrs}
        for row in context.table
    ]
    assert_sorted_equal(expected, found, "Repository list")
