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

.. _systemstate_misc_ref-label:

##############
 System state
##############

Description
===========

The DNF5 system state consists of several TOML files with their location determined by the `system_state_dir` configuration option (:manpage:`dnf5-conf(5)`, :ref:`system_state_dir <system_state_dir_options-label>`). DNF5 uses the system state to:

    1. Store the reasons why each installed package was added to the system. The reasons can be "user" for packages that the user explicitly asked DNF5 to install, "dependency" and "weak dependency" for packages pulled in as dependencies of another package, "group" for packages installed by a group, or "external" for packages installed by another tool (e.g. rpm).

    2. Track installed groups and packages installed by these groups.

    3. Track installed environmental groups.


The way of storing the DNF5 system state is an internal implementation detail and may change at any time. To modify the state, always use the DNF5 command-line interface or DNF5 API.


Recovering from a corrupted system state
========================================

If the system state files become corrupted, simply back up and remove the corrupted TOML file mentioned in the error message. It will be regenerated during the next successful DNF5 transaction.
The regenerated files may lack some data, such as the reasons why packages were installed or the repositories from which they were installed.
