.. Copyright Contributors to the DNF5 project.
.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

###################################################
 Package management service with a D-Bus interface
###################################################


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
