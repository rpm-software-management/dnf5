# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import dbus
import os

DNFDAEMON_BUS_NAME = 'org.rpm.dnf.v0'
DNFDAEMON_OBJECT_PATH = '/' + DNFDAEMON_BUS_NAME.replace('.', '/')

IFACE_SESSION_MANAGER = '{}.SessionManager'.format(DNFDAEMON_BUS_NAME)
IFACE_REPO = '{}.rpm.Repo'.format(DNFDAEMON_BUS_NAME)
IFACE_REPOCONF = '{}.rpm.RepoConf'.format(DNFDAEMON_BUS_NAME)
IFACE_RPM = '{}.rpm.Rpm'.format(DNFDAEMON_BUS_NAME)
IFACE_GOAL = '{}.Goal'.format(DNFDAEMON_BUS_NAME)
IFACE_HISTORY = '{}.History'.format(DNFDAEMON_BUS_NAME)
IFACE_ADVISORY = '{}.Advisory'.format(DNFDAEMON_BUS_NAME)
IFACE_GROUP = '{}.comps.Group'.format(DNFDAEMON_BUS_NAME)


def dbus_to_python(data):
    """Recursively convert dbus types to plain python types.
    Covers types from
    https://dbus.freedesktop.org/doc/dbus-python/dbus.types.html (not all)"""
    if isinstance(data, dbus.Dictionary):
        return {dbus_to_python(k): dbus_to_python(v) for k, v in data.items()}
    if isinstance(data, dbus.Array):
        return [dbus_to_python(i) for i in data]
    if isinstance(data, dbus.Struct):
        return tuple(dbus_to_python(i) for i in data)
    if isinstance(data, (dbus.String, dbus.ObjectPath, dbus.Signature)):
        return str(data)
    if isinstance(data, (dbus.Byte, dbus.Int16, dbus.Int32, dbus.Int64,
                         dbus.UInt16, dbus.UInt32, dbus.UInt64)):
        return int(data)
    if isinstance(data, dbus.Boolean):
        return bool(data)
    if isinstance(data, dbus.Double):
        return float(data)

    return data


def convert_table_value(value, template):
    """Convert a string from a behave table cell to the type of template."""
    if isinstance(template, bool):
        return value == "True"
    if isinstance(template, int):
        return int(value)
    return value


class DbusSession(object):
    """A dnf5daemon session owned by the current D-Bus connection.

    The session has to be used from a single D-Bus connection - the daemon
    only allows session method calls for the D-Bus sender that opened the
    session. Therefore one dbus.SystemBus() is kept for the whole session
    lifetime (which spans multiple behave steps of a scenario).
    """

    def __init__(self, context, session_config=None):
        self.bus = dbus.SystemBus()
        self.session_manager = dbus.Interface(
            self.bus.get_object(DNFDAEMON_BUS_NAME, DNFDAEMON_OBJECT_PATH),
            dbus_interface=IFACE_SESSION_MANAGER)

        if session_config is None:
            # default session configuration for the D-Bus API tests
            session_config = {
                "config": {
                    "installroot": context.dnf.installroot,
                    "cachedir": os.path.join(context.dnf.installroot, "var/cache/dnf"),
                    # prevent loading plugins installed on the host
                    # (https://github.com/rpm-software-management/dnf5/issues/1316)
                    "plugins": "False",
                }
            }

        self.object_path = self.session_manager.open_session(session_config)
        session_object = self.bus.get_object(DNFDAEMON_BUS_NAME, self.object_path)
        self.repo = dbus.Interface(session_object, dbus_interface=IFACE_REPO)
        self.rpm = dbus.Interface(session_object, dbus_interface=IFACE_RPM)
        self.goal = dbus.Interface(session_object, dbus_interface=IFACE_GOAL)

        # resolved transaction (sanitized) and the resolve result code
        self.resolved = None
        self.result = None
        # last caught DBusException
        self.error = None
        # return value of the last close_session call
        self.close_result = None
        self.closed = False

    def close(self):
        """Close the session. Returns the close_session return value."""
        if self.closed:
            return self.close_result
        self.close_result = self.session_manager.close_session(self.object_path)
        self.closed = True
        return self.close_result

    def resolve(self):
        """Resolve the goal. Stores the transaction and the result."""
        resolved, result = self.goal.resolve(dbus.Dictionary({}, signature='sv'))
        self.resolved = resolved
        self.result = result
        return result

    def rpm_action(self, method, specs, options=None):
        """Call one of the Rpm interface methods (install, remove, upgrade,
        downgrade, reinstall, distro_sync) with a list of package specs."""
        if options is None:
            options = {}
        iface_method = getattr(self.rpm, method)
        iface_method(list(specs), dbus.Dictionary(options, signature='sv'))
