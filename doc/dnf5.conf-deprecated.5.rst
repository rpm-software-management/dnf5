.. Copyright Contributors to the DNF5 project.
..
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

.. _dnf5_conf_deprecated-label:

Config Options that are deprecated in DNF5
===========================================

`DNF5` has some options that are deprecated.
This section documents the options that are deprecated and will be removed in the future.

If you find any issue, please, open a ticket at https://github.com/rpm-software-management/dnf5/issues.


    .. WARNING::
       All config options documented here are deprecated.

[main] Options
==============

.. _strict_options-label:

``strict``
    :ref:`boolean <boolean-label>`

    If disabled, all unavailable packages or packages with broken dependencies given to DNF5
    command will be skipped without raising the error causing the whole operation to fail.
    Currently works for install command only.

    Default: ``True``.


Options for both [main] and Repo
================================

.. _deltarpm_options-label:

``deltarpm``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will save bandwidth by downloading much smaller delta RPM
    files, rebuilding them to RPM locally. However, this is quite CPU and I/O
    intensive.

    Default: ``False``.

.. _deltarpm_percentage_options-label:

``deltarpm_percentage``
    :ref:`integer <integer-label>`

    When the relative size of delta vs pkg is larger than this, delta is not used.
    (Deltas must be at least 25% smaller than the pkg).
    Use ``0`` to turn off delta rpm processing. Local repositories (with
    file:// baseurl) have delta rpms turned off by default.

    Default: ``75``

.. _retries_options-label:

``retries``
    :ref:`integer <integer-label>`

    Set the number of total retries for downloading packages.
    The number is cumulative, so e.g. for ``retries=10``, DNF5 will fail after any package
    download fails for eleventh time.

    Setting this to ``0`` makes DNF5 try forever.

    Default: ``10``.
