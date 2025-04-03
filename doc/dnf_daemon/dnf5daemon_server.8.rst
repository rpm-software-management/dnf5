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


##################################################
 Package management service with a DBus interface
##################################################


Synopsis
========

``systemctl start dnf5daemon-server``


Description
===========

dnf5daemon-server is D-Bus interface for libdnf5 package manager.


Files
=====

``dnf5daemon-server configuration``
    /etc/dnf/dnf5daemon-server.conf

    Use the ``[main]`` section to override any main DNF5 configuration option. See :manpage:`dnf5.conf(5)`, :ref:`Main configuration options <conf_main_options-label>`.
