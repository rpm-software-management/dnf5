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

.. _expired-pgp-keys_plugin_ref-label:

########################
 Expired PGP Keys Plugin
########################

Description
===========

The plugin checks for installed but expired PGP keys before executing the transaction.
For each expired key, the user is prompted with information about the specific key
and can confirm its removal, allowing for the import of an updated key later.
When the ``assumeyes`` option is configured, expired keys are removed automatically.

Configuration
=============

The plugin configuration is in ``/etc/dnf/libdnf5-plugins/expired-pgp-keys.conf``. All configuration
options are in the ``[main]`` section.

``enabled``
    Whether the plugin is enabled. Default value is ``True``.
