.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

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
