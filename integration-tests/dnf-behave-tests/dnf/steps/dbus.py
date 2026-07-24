# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave

from cmd import execute_python_script

DBUS_SESSION_SETUP = """
import dbus

DNFDAEMON_BUS_NAME = 'org.rpm.dnf.v0'
DNFDAEMON_OBJECT_PATH = '/' + DNFDAEMON_BUS_NAME.replace('.', '/')

IFACE_SESSION_MANAGER = '{{}}.SessionManager'.format(DNFDAEMON_BUS_NAME)
IFACE_REPO = '{{}}.rpm.Repo'.format(DNFDAEMON_BUS_NAME)
IFACE_REPOCONF = '{{}}.rpm.RepoConf'.format(DNFDAEMON_BUS_NAME)
IFACE_RPM = '{{}}.rpm.Rpm'.format(DNFDAEMON_BUS_NAME)
IFACE_GOAL = '{{}}.Goal'.format(DNFDAEMON_BUS_NAME)
IFACE_HISTORY = '{{}}.History'.format(DNFDAEMON_BUS_NAME)
IFACE_ADVISORY = '{{}}.Advisory'.format(DNFDAEMON_BUS_NAME)
IFACE_GROUP = '{{}}.comps.Group'.format(DNFDAEMON_BUS_NAME)


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
        if "transaction_time" in pkg:
            print("Transaction time: {{}}".format(int(pkg["transaction_time"])))
        if "advisories" in pkg:
            print("Advisories: {{}}".format(", ".join(pkg["advisories"])))

def print_recent_history(dbus_output):
    for key in ["installed", "upgraded", "downgraded", "removed"]:
        pkgs = dbus_output.get(key, [])
        print("{{}}: {{}}".format(key, len(pkgs)))
        print_recent_history_pkgs(pkgs)

def dbus_to_python(data):
    # Covers types from https://dbus.freedesktop.org/doc/dbus-python/dbus.types.html (not all)
    if isinstance(data, dbus.Dictionary):
        return {{dbus_to_python(k): dbus_to_python(v) for k, v in data.items()}}
    elif isinstance(data, dbus.Array):
        return [dbus_to_python(i) for i in data]
    elif isinstance(data, (dbus.String, dbus.ObjectPath, dbus.Signature)):
        return str(data)
    elif isinstance(data, (dbus.Int16, dbus.Int32, dbus.Int64, dbus.UInt16, dbus.UInt32, dbus.UInt64)):
        return int(data)
    elif isinstance(data, dbus.Byte):
        return int(data)
    elif isinstance(data, dbus.Boolean):
        return bool(data)
    elif isinstance(data, dbus.Double):
        return float(data)

    return data

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
