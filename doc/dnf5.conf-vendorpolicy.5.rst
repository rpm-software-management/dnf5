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

The configuration files use TOML format.

File Structure
==============

Required Fields
---------------

``version``
    String

    Configuration file format version.

    Supported values: ``"1.0"``

    This field is mandatory and must be specified at the top level of the TOML file.

    Example::

        version = '1.0'

Vendor Mapping Definition
--------------------------

The file must contain **one** of the following definition methods:

**Option 1: Separate Lists (outgoing + incoming)**

``[[outgoing_vendors]]``
    Array of tables

    List of vendors from which changes are allowed.

``[[incoming_vendors]]``
    Array of tables

    List of vendors to which changes are allowed.

    .. NOTE::
       ``outgoing_vendors`` and ``incoming_vendors`` must have the same state - either both empty or both non-empty.

**Option 2: Equivalent Vendors**

``[[equivalent_vendors]]``
    Array of tables

    List of vendors that are mutually equivalent. Changes are allowed in both directions
    between all vendors in this list.

    .. NOTE::
       Cannot combine ``equivalent_vendors`` with ``outgoing_vendors`` or ``incoming_vendors``.
       However, ``equivalent_vendors`` can be replaced by specifying the same list of vendors
       in both ``outgoing_vendors`` and ``incoming_vendors``.

Vendor Entry Fields
-------------------

Each entry in ``[[outgoing_vendors]]``, ``[[incoming_vendors]]``, or ``[[equivalent_vendors]]``
can contain the following fields:

``vendor``
    String

    Required field.

    Vendor name or pattern for matching.

    Example::

        vendor = 'Fedora*'

``comparator``
    String

    Optional field.

    The matching method to use when comparing vendor strings.

    Default: ``"EXACT"``

    Supported values:

    - ``"EXACT"`` - exact match (case-sensitive)
    - ``"IEXACT"`` - exact match (case-insensitive)
    - ``"GLOB"`` - glob pattern (case-sensitive)
    - ``"IGLOB"`` - glob pattern (case-insensitive)
    - ``"REGEX"`` - regular expression (case-sensitive)
    - ``"IREGEX"`` - regular expression (case-insensitive)
    - ``"CONTAINS"`` - contains string (case-sensitive)
    - ``"ICONTAINS"`` - contains string (case-insensitive)
    - ``"STARTSWITH"`` - starts with (case-sensitive)
    - ``"ISTARTSWITH"`` - starts with (case-insensitive)
    - ``"ENDSWITH"`` - ends with (case-sensitive)
    - ``"IENDSWITH"`` - ends with (case-insensitive)
    - ``"NOT_EXACT"``, ``"NOT_IEXACT"``, ``"NOT_GLOB"``, ``"NOT_IGLOB"``, ``"NOT_CONTAINS"``, ``"NOT_ICONTAINS"`` - negated variants

    Example::

        comparator = 'IGLOB'

``exclude``
    Boolean

    Optional field.

    If ``true``, the vendor is excluded from the rule. This is useful for defining
    exceptions to more general rules. Rules (vendor entries) are processed in the order
    they are defined. This means that an exclude rule must appear before the rules
    from which the vendor should be excluded.

    Default: ``false``

    Example::

        exclude = true

Error Conditions
================

The following configurations are **invalid** and will cause an error:

- Missing ``version`` field
- Incorrect version (other than ``"1.0"``)
- Combination of ``equivalent_vendors`` with ``outgoing_vendors`` or ``incoming_vendors``
- Only ``outgoing_vendors`` without ``incoming_vendors`` (or vice versa)
- Missing required ``vendor`` field in an entry
- Unknown ``comparator`` value
- Unknown keys at the top level or inside vendor entries

Configuration File Locations
=============================

Vendor change policy files are read from the following directories:

``/etc/dnf/vendors.d/``
    System configuration directory

``/usr/share/dnf5/vendors.d/``
    Distribution configuration directory

If a file with the same name exists in both directories, the file from ``/etc/dnf/vendors.d/``
is used. This implies that the distribution configuration file can be simply overridden
by creating a file with the same name in the ``/etc/dnf/vendors.d/`` directory.

Examples
========

Example 1: Allow change from "VendorA" to "VendorB"
---------------------------------------------------

This example shows the minimal required configuration, allowing a change from
"VendorA" to "VendorB", but not the reverse.

.. code-block:: toml

    version = '1.0'

    [[outgoing_vendors]]
    vendor = 'VendorA'

    [[incoming_vendors]]
    vendor = 'VendorB'

Example 2: Allow change from any vendor to "My Trusted Vendor"
--------------------------------------------------------------

This example shows the minimal required configuration, allowing a change from
any vendor to "My Trusted Vendor", but not the reverse.

.. code-block:: toml

    version = '1.0'

    [[outgoing_vendors]]
    vendor = ''
    comparator = 'CONTAINS'

    [[incoming_vendors]]
    vendor = 'My Trusted Vendor'

Example 3: Equivalent vendors
-----------------------------

This example shows vendors that are mutually equivalent, allowing changes
in both directions.

.. code-block:: toml

    version = '1.0'

    # All following vendors are mutually equivalent
    [[equivalent_vendors]]
    vendor = 'Fedora Project'

    [[equivalent_vendors]]
    vendor = 'Red Hat'
    comparator = 'ISTARTSWITH'

    [[equivalent_vendors]]
    vendor = 'CentOS'
    comparator = 'ISTARTSWITH'

Example 4: Equivalent vendors with an exclusion
-----------------------------------------------

This example shows a vendor policy for SUSE-related vendors with an exclusion
for openSUSE Build Service.

.. code-block:: toml

    version = '1.0'

    # Vendors from which changes are allowed
    [[equivalent_vendors]]
    vendor = 'openSUSE Build Service'
    comparator = 'ISTARTSWITH'
    exclude = true

    [[equivalent_vendors]]
    vendor = 'SUSE'
    comparator = 'ISTARTSWITH'

    [[equivalent_vendors]]
    vendor = 'openSUSE'
    comparator = 'ISTARTSWITH'

See Also
========

* :manpage:`dnf5.conf(5)`, :ref:`DNF5 Configuration Reference <dnf5_conf-label>`
