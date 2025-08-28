.. Copyright Contributors to the DNF5 project.
.. Copyright Contributors to the libdnf project.
.. SPDX-License-Identifier: GPL-2.0-or-later

.. _makecache_command_ref-label:

##################
 Makecache Command
##################

Synopsis
========

``dnf5 makecache [global options]``


Description
===========

The ``makecache`` command in ``DNF5`` is used for creating and downloading metadata
for enabled repositories.

It tries to avoid downloading whenever possible, e.g. when the local metadata hasn't
expired yet or when the metadata timestamp hasn't changed.
