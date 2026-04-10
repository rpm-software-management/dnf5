..
    Copyright Contributors to the DNF5 project
    SPDX-License-Identifier: LGPL-2.1-or-later

.. _local_plugin_ref-label:

############
Local Plugin
############

Description
===========

After each libdnf5 transaction copy all downloaded packages to local repositories on the filesystem and generate repository metadata.

The plugin creates two repositories:

* ``_dnf_local`` for packages from repositories with ``pkg_gpgcheck`` enabled.
  The ``pkg_gpgcheck`` setting is inherited from the main configuration.
  The repository doesn't specify any ``gpgkey``, it assumes all required keys
  were already imported.
* ``_dnf_local_nogpgcheck`` for packages from repositories with ``pkg_gpgcheck``
  disabled. This repository has ``pkg_gpgcheck`` set to ``false``.

Each repository is only created when there are cached packages in it.

Both repositories are configured with the following options::

    skip_if_unavailable = true
    metadata_expire = 0

The ``_dnf_local`` repository has ``cost = 500`` and the ``_dnf_local_nogpgcheck``
repository has ``cost = 501``, so the gpgcheck-enabled repository is preferred
when both contain the same package.

To generate the metadata `createrepo_c` is required.

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

``repodir_nogpgcheck``
    :ref:`string <string-label>`

    Path where the local repository for packages from repos with ``pkg_gpgcheck``
    disabled is located.
    By default it is in :ref:`persistdir <persistdir_options-label>` in ``plugins/local-nogpgcheck`` subdirectory.


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
