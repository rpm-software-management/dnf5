..
    Copyright Contributors to the DNF5 project
    SPDX-License-Identifier: LGPL-2.1-or-later

.. _local_plugin_ref-label:

############
Local Plugin
############

Description
===========

After each libdnf5 transaction copy all downloaded packages to a ``_dnf_local`` repository on the local filesystem and generate repository metadata.

The repository is automatically added by the plugin with the following options::

    [_dnf_local]
    name = Local libdnf5 plugin repo
    baseurl = <repodir>
    skip_if_unavailable = true
    cost = 500
    metadata_expire = 0

Note that the repository has ``pkg_gpgcheck`` verification enabled by default but doesn't specify any ``gpgkey``, it assumes all required keys were already imported.

To generate the metedata `createrepo_c` is required.

Configuration
=============

The plugin configuration is in ``/etc/dnf/libdnf5-plugins/local.conf``.
The minimal content of the conf file should contain ``main`` and ``createrepo`` sections.

The ``main`` section requires:

``name``
    :ref:`string <string-label>`

    The plugin's name is ``local``

``enabled``
    Whether or where the plugin is enabled: ``true``, ``false``, ``host-only``, ``installroot-only``.

    * For ``host-only`` the plugin will run only on transactions in the default installroot ``/``.
    * For ``installroot-only`` the plugin will run only on transactions in installroots different from ``/``.

It can also contain:

``repodir``
    :ref:`string <string-label>`

    Path where the local repository is located.
    By default it is in :ref:`persistdir <persistdir_options-label>` in ``plugins/local`` subdirectory.


The ``createrepo`` section requires:

``enabled``
    :ref:`boolean <boolean-label>`

    Whether running ``createrepo`` to generate repodata is enabled: ``true`` or ``false``.

Additionally it can contain:

``cachedir``
    :ref:`string <string-label>`

    If you want to speed up ``createrepo`` with the ``--cachedir`` option.
    Not used by default.

``quiet``
    :ref:`boolean <boolean-label>`

    Whether to run ``createrepo`` with ``--quiet`` option. On by default.

``verbose``
    :ref:`boolean <boolean-label>`

    Whether to run ``createrepo`` with ``--verbose`` option. Off by default.
