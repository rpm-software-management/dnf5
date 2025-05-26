..
    Copyright Contributors to the libdnf project.

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

.. only:: sphinx4

   ..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.SessionManager.xml

.. only:: not sphinx4

   .. warning::
      Sphinx 4 is required to build D-Bus documentation.

      This is the content of ``dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.SessionManager.xml``:

   .. literalinclude:: ../../dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.SessionManager.xml
      :language: xml

.. only:: sphinx4

   ..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Base.xml

.. only:: not sphinx4

   .. warning::
      Sphinx 4 is required to build D-Bus documentation.

      This is the content of ``dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Base.xml``:

   .. literalinclude:: ../../dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Base.xml
      :language: xml

.. only:: sphinx4

   ..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.rpm.Repo.xml

.. only:: not sphinx4

   .. warning::
      Sphinx 4 is required to build D-Bus documentation.

      This is the content of ``dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.rpm.Repo.xml``:

   .. literalinclude:: ../../dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.rpm.Repo.xml
      :language: xml


.. only:: sphinx4

   ..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.rpm.Rpm.xml

.. only:: not sphinx4

   .. warning::
      Sphinx 4 is required to build D-Bus documentation.

      This is the content of ``dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.rpm.Rpm.xml``:

   .. literalinclude:: ../../dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.rpm.Rpm.xml
      :language: xml

.. only:: sphinx4

   ..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Goal.xml


.. only:: not sphinx4

   .. warning::
      Sphinx 4 is required to build D-Bus documentation.

      This is the content of ``dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Goal.xml``:

   .. literalinclude:: ../../dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Goal.xml
      :language: xml


.. only:: sphinx4

   ..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Offline.xml


.. only:: not sphinx4

   .. warning::
      Sphinx 4 is required to build D-Bus documentation.

      This is the content of ``dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Offline.xml``:

   .. literalinclude:: ../../dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Offline.xml
      :language: xml



.. only:: sphinx4

   ..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.comps.Group.xml

.. only:: not sphinx4

   .. warning::
      Sphinx 4 is required to build D-Bus documentation.

      This is the content of ``dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.comps.Group.xml``:

   .. literalinclude:: ../../dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.comps.Group.xml
      :language: xml


.. only:: sphinx4

   ..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Advisory.xml

.. only:: not sphinx4

   .. warning::
      Sphinx 4 is required to build D-Bus documentation.

      This is the content of ``dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Advisory.xml``:

   .. literalinclude:: ../../dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.Advisory.xml
      :language: xml


.. only:: sphinx4

   ..  dbus-doc:: dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.History.xml

.. only:: not sphinx4

   .. warning::
      Sphinx 4 is required to build D-Bus documentation.

      This is the content of ``dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.History.xml``:

   .. literalinclude:: ../../dnf5daemon-server/dbus/interfaces/org.rpm.dnf.v0.History.xml
      :language: xml
