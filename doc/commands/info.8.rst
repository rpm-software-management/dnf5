.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

.. _info_command_ref-label:

#############
 Info Command
#############

Synopsis
========

``dnf5 info [options] [<package-spec-NI>...]``


Description
===========

Prints detailed information about packages based on the provided parameters.


Options
=======

``--showduplicates``
    | Show all versions of the packages, not only the latest one.

.. include:: ../_shared/options/installed-from-repo.rst

``--installed``
    | List only installed packages.

``--available``
    | List only available packages.

``--extras``
    | List only extras: packages installed on the system that are not available in any known repository.

``--obsoletes``
    | List only packages installed on the system that are obsoleted by packages in any known repository.

``--recent``
    | List only packages recently added to the repositories.

``--upgrades``
    | List only available upgrades for installed packages.

``--autoremove``
    | List only packages that will be removed by the :ref:`autoremove command <autoremove_command_ref-label>`.


Examples
========

``dnf5 info``
    | Display detailed information about installed and available packages.

``dnf5 info --recent gnome*``
    | Print information about recent packages whose names start with ``gnome``.


See Also
========

    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`
