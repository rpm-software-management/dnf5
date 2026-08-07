..
    Copyright Contributors to the DNF5 project.
    SPDX-License-Identifier: GPL-2.0-or-later

.. _dnf5_vendor_change_policy-label:

##########################################
 DNF5 Vendor Change Policy File Reference
##########################################

Description
===========

The vendor change policy configuration files define rules for allowed package vendor changes.
They specify which vendors (outgoing) are allowed to change to which other vendors (incoming).
Vendor matching is performed during package replacement (upgrade/downgrade/reinstall) operations
when the :ref:`allow_vendor_change <allow_vendor_change_options-label>` option is disabled.

The configuration files use **TOML format**.

Supported Versions
==================

The vendor change policy configuration format supports three versions:

- **Version 1.2** - Extends v1.1 with additional negated string comparators. See :ref:`dnf5_vendor_change_policy_v1_1-label`
- **Version 1.1** - Enhanced version with flexible vendor mapping and package filtering. See :ref:`dnf5_vendor_change_policy_v1_1-label`
- **Version 1.0** - Basic vendor mapping with strict rules. See :ref:`dnf5_vendor_change_policy_v1_0-label`

Each configuration file must specify its version using the ``version`` field.

.. note::
   Each newer version is fully backward compatible with all previous versions.
   Version 1.2 only adds new comparators over 1.1, and version 1.1 adds flexible
   vendor mapping and package filtering over 1.0. All valid configurations work
   with a newer version by simply changing the version field.
   Version 1.2 is the recommended format.

For detailed information about each version's syntax and features, please refer to the
version-specific documentation:

- :ref:`DNF5 Vendor Change Policy File Reference - v1.1 / v1.2 <dnf5_vendor_change_policy_v1_1-label>`
- :ref:`DNF5 Vendor Change Policy File Reference - v1.0 <dnf5_vendor_change_policy_v1_0-label>`

Configuration File Locations
============================

Vendor change policy files are read from the following directories:

``/etc/dnf/vendors.d/``
    System configuration directory

``/usr/share/dnf5/vendors.d/``
    Distribution configuration directory

Only files with the ``.conf`` extension are considered. Configuration files are sorted
alphanumerically by their filename and then read in this sorted order.

If a file with the same name exists in both directories, the file from ``/etc/dnf/vendors.d/``
is used. This implies that the distribution configuration file can be simply overridden
by creating a file with the same name in the ``/etc/dnf/vendors.d/`` directory.

Command-Line Policies
=====================

Vendor change policies can also be specified directly on the command line using
the ``--add-vendor-policy`` option with a compact syntax. This is useful for
adding policies for a single run without creating configuration files.

The ``--clear-vendor-policies`` option can be used to remove all policies
loaded from configuration files, optionally replacing them with command-line policies.

For the compact format syntax and examples, see
:ref:`Compact Format <vendorpolicy_compact_format-label>` in the v1.1 reference.

See Also
========

* :manpage:`dnf5.conf-vendorpolicy-v1_1(5)`, :ref:`DNF5 Vendor Change Policy File Reference - v1.1 / v1.2 <dnf5_vendor_change_policy_v1_1-label>`
* :manpage:`dnf5.conf-vendorpolicy-v1_0(5)`, :ref:`DNF5 Vendor Change Policy File Reference - v1.0 <dnf5_vendor_change_policy_v1_0-label>`
* :manpage:`dnf5.conf(5)`, :ref:`DNF5 Configuration Reference <dnf5_conf-label>`
