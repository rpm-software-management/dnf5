..
    Copyright Contributors to the DNF5 project.
    SPDX-License-Identifier: GPL-2.0-or-later

.. _dnf5_vendor_change_policy_v1_0-label:

##################################################
 DNF5 Vendor Change Policy File Reference - v1.0
##################################################

Description
===========

This document describes the structure and syntax of the DNF5 Vendor Change Policy
configuration files using **version 1.0** format.

For general information about vendor change policy files, see :ref:`dnf5_vendor_change_policy-label`.

File Structure
==============

Required Fields
---------------

``version``
    String

    Configuration file format version.

    For this format, must be set to ``"1.0"``.

    This field is mandatory and must be specified at the top level of the TOML file.

    Example::

        version = '1.0'

.. NOTE::
   For the configuration file to have any effect on vendor change policy, it must define
   vendor mappings using one of the allowed configurations (see below).

   If the file contains no vendor lists, it will be loaded and its version will be validated,
   but it will not define any rules and will not affect vendor manager behavior.

Vendor Mapping Definition
--------------------------

The file can use one of the following vendor definition methods.
**Only one method can be used**.

**Option 1: Separate Lists (outgoing + incoming)**

``[[outgoing_vendors]]``
    Array of tables

    List of vendors from which changes are allowed (outgoing package vendors).

``[[incoming_vendors]]``
    Array of tables

    List of vendors to which changes are allowed (incoming package vendors).

.. NOTE::
   ``outgoing_vendors`` and ``incoming_vendors`` must be either both present or both missing.

.. IMPORTANT::
   A vendor change during package replacement is allowed **only if both conditions
   are met simultaneously**:

   - The vendor of the outgoing (installed) package is listed in ``outgoing_vendors``
   - The vendor of the incoming (new) package is listed in ``incoming_vendors``

**Option 2: Equivalent Vendors**

``[[equivalent_vendors]]``
    Array of tables

    List of vendors that are mutually equivalent. Changes are allowed in both directions
    between all vendors in this list.

    .. NOTE::
       Cannot combine ``equivalent_vendors`` with ``outgoing_vendors`` or ``incoming_vendors``.
       However, ``equivalent_vendors`` is actually a shorthand notation. It is equivalent to
       listing the same vendors in both ``outgoing_vendors`` and ``incoming_vendors``.

Vendor Entry Fields
-------------------

Each entry in ``[[outgoing_vendors]]``, ``[[incoming_vendors]]``, or ``[[equivalent_vendors]]``
can contain the following fields:

``vendor``
    String

    Required field.

    Vendor name or pattern for matching.

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

``exclude``
    Boolean

    Optional field.

    If ``true``, the vendor is excluded from the rule. This is useful for defining
    exceptions to more general rules. Rules (vendor entries) are processed in the order
    they are defined. This means that an exclude rule must appear before the rules
    from which the vendor should be excluded.

    Default: ``false``

Error Conditions
================

The following configurations are **invalid** and will cause an error:

- Missing ``version`` field
- Combination of ``equivalent_vendors`` with ``outgoing_vendors`` or ``incoming_vendors``
- Only ``outgoing_vendors`` without ``incoming_vendors`` (or vice versa)
- Missing required ``vendor`` field in a vendor entry
- Unknown ``comparator`` value
- Invalid regex pattern in vendor value
- Unknown keys at the top level or inside entries

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

This example shows allowing a change from any vendor to "My Trusted Vendor",
but not the reverse (requires explicit rule for allowing all vendors).

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

    # All following vendors are mutually equivalent except excluded ones
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

* :ref:`DNF5 Vendor Change Policy File Reference <dnf5_vendor_change_policy-label>`
* :ref:`DNF5 Vendor Change Policy File Reference - v1.1 <dnf5_vendor_change_policy_v1_1-label>`
* :manpage:`dnf5.conf(5)`, :ref:`DNF5 Configuration Reference <dnf5_conf-label>`
