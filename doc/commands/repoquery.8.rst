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

.. _repoquery_command_ref-label:

##################
 Repoquery Command
##################

Synopsis
========

``dnf5 repoquery [options] [<spec>...]``


Description
===========

The ``repoquery`` command in ``DNF5`` is used for querying packages by matching
various input criteria from the user. Arguments defined in ``spec`` list could be
either file paths or package specs.


Options
=======

``--advisories=ADVISORY_NAME,...``
    | Consider only content contained in advisories with specified name.
    | This is a list option.
    | Expected values are advisory IDs, e.g. `FEDORA-2201-123`.

``--advisory-severities=ADVISORY_SEVERITY,...``
    | Consider only content contained in advisories with specified severity. 
    | This is a list option. 
    | Accepted values are: `critical`, `important`, `moderate`, `low`, `none`.

``--bzs=BUGZILLA_ID,...``
    | Consider only content contained in advisories that fix a ticket of given Bugzilla ID. 
    | This is a list option.
    | Expected values are numeric IDs, e.g. `123123`.

``--cves=CVE_ID,...``
    | Consider only content contained in advisories that fix a ticket of given CVE (Common Vulnerabilities and Exposures) ID.
    | This is a list option.
    | Expected values are string IDs in CVE format, e.g. `CVE-2201-0123`. 

``--security``
    | Consider only content contained in security advisories.

``--bugfix``
    | Consider only content contained in bugfix advisories.

``--enhancement``
    | Consider only content contained in enhancement advisories.

``--newpackage``
    | Consider only content contained in newpackage advisories.

``--available``
    | Display all available packages.
    | This is the default behavior.

``--installed``
    | Display only installed packages.

``--nevra``
    | Use the `NEVRA` format for the output.
    | This is the default behavior.
    | This can be useful for scripting or automation as the output is in machine-readable format.

``--info``
    | Use the verbose format for the output.


Examples
========

``dnf5 repoquery /etc/koji.conf``
    | List packages which provide the given file.

``dnf5 repoquery *http*``
    | List packages containing the ``http`` inside their name.

``dnf5 repoquery --installed --security``
    | List installed packages included in any security advisories.


See Also
========

    | :manpage:`dnf5-advisory(8)`, :ref:`Advisory command <advisory_command_ref-label>`
    | :manpage:`dnf5-specs(7)`, :ref:`Patterns specification <specs_misc_ref-label>`

