..
    Copyright Contributors to the DNF5 project.
    Copyright Contributors to the libdnf project.
    SPDX-License-Identifier: GPL-2.0-or-later

    This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

    Libdnf is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    Libdnf is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

##################################
 D-Bus API bindings for dnfdaemon
##################################


Synopsis
========

D-Bus methods and arguments. Generated from D-Bus XMLs.


Description
===========

Note: While most of the following methods can be invoked successfully by a regular user, authentication by an administrative user is required for some of them (e.g. `Goal.do_transaction()`, `Repo.confirm_key()`, `Repo.enable()`, `Repo.disable()`). This authentication is managed by calling `CheckAuthorization()` method of the `org.freedesktop.PolicyKit1.Authority` Polkit D-Bus interface. The `AllowUserInteraction` flag is set for this call, indicating that if an authentication agent is available, the call is blocked while the user is prompted to authenticate. A hardcoded timeout of 2 minutes is set for the user interaction.

Interfaces
==========

..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.SessionManager.xml

..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Base.xml

..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.rpm.Repo.xml

..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.rpm.Rpm.xml

..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Goal.xml

..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Offline.xml

..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.comps.Group.xml

..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Advisory.xml

..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.History.xml
