..
    Copyright Contributors to the DNF5 project.
    SPDX-License-Identifier: GPL-2.0-or-later

.. _dnf5_vendor_change_policy_v1_1-label:

##################################################
 DNF5 Vendor Change Policy File Reference - v1.1
##################################################

Description
===========

This document describes the structure and syntax of the DNF5 Vendor Change Policy
configuration files using **version 1.1** format.

For general information about vendor change policy files, see :ref:`dnf5_vendor_change_policy-label`.

File Structure
==============

Required Fields
---------------

``version``
    String

    Configuration file format version.

    For this format, must be set to ``"1.1"``.

    This field is mandatory and must be specified at the top level of the TOML file.

    Example::

        version = '1.1'

.. NOTE::
   For the configuration file to have any effect on vendor change policy, it must contain
   at least one of the supported lists (see below): ``outgoing_vendors``, ``incoming_vendors``,
   ``equivalent_vendors``, ``outgoing_packages``, or ``incoming_packages``.

   If the file contains no lists, it will be loaded and its version will be validated,
   but it will not define any rules and will not affect vendor manager behavior.

Vendor Mapping Definition
--------------------------

The following vendor lists can be used to define vendor mappings.
All lists are optional and can be freely combined. All conditions are evaluated together.

``[[outgoing_vendors]]``
    Array of tables

    List of vendors from which changes are allowed (outgoing package vendors).

``[[incoming_vendors]]``
    Array of tables

    List of vendors to which changes are allowed (incoming package vendors).

``[[equivalent_vendors]]``
    Array of tables

    List of vendors that are mutually equivalent. Changes are allowed in both directions
    between all vendors in this list.

    .. NOTE::
       ``equivalent_vendors`` is actually a shorthand notation. It is equivalent to
       listing the same vendors in both ``outgoing_vendors`` and ``incoming_vendors``.

.. IMPORTANT::
   A vendor change during package replacement is allowed **only if both conditions
   are met simultaneously**:

   - The vendor of the outgoing (installed) package is listed in ``outgoing_vendors``
     or ``equivalent_vendors`` (or both ``outgoing_vendors`` and ``equivalent_vendors``
     are missing, which allows any vendor)
   - The vendor of the incoming (new) package is listed in ``incoming_vendors``
     or ``equivalent_vendors`` (or both ``incoming_vendors`` and ``equivalent_vendors``
     are missing, which allows any vendor)

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

Package Filtering
-----------------

Vendor change policies can be further restricted to apply only to specific packages
using package filters. These are optional and allow fine-grained control over which
packages are allowed to change vendors.

``[[outgoing_packages]]``
    Array of tables

    List of package filters that restrict which outgoing (installed) packages are
    allowed to change vendors. If omitted, all packages are allowed to change from
    their current vendor.

``[[incoming_packages]]``
    Array of tables

    List of package filters that restrict which incoming (new) packages are allowed
    to be installed with a vendor change. If omitted, all packages are allowed to
    be installed regardless of vendor.

Package Filter Entry Fields
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each entry in ``[[outgoing_packages]]`` or ``[[incoming_packages]]`` can contain:

``filters``
    Array of tables

    Required field.

    List of filter conditions that must all match for the package to be affected by
    this rule. Each filter is a table with the following fields:

    ``filter``
        String

        Required field.

        The package attribute to filter on.

        Supported values:

        - ``"name"`` - package name
        - ``"source_name"`` - source package name
        - ``"evr"`` - Epoch-Version-Release string
        - ``"epoch"`` - epoch number
        - ``"version"`` - version string
        - ``"release"`` - release string
        - ``"arch"`` - package architecture
        - ``"repoid"`` - repository ID
        - ``"cmdline_repo"`` - whether the package is from command-line repository (boolean)

    ``value``
        String

        Required field.

        The value to match against.

        - For ``cmdline_repo`` filter, use ``"true"``/``"1"`` or ``"false"``/``"0"``
        - For ``epoch`` filter, use a numeric string (e.g., ``"0"``, ``"1"``)
        - For other filters, use appropriate string values

    ``comparator``
        String

        Optional field.

        The matching method to use.

        Default: ``"EXACT"``

        **For string-based filters** (``name``, ``source_name``, ``arch``, ``repoid``):

        Same values as for vendor comparator: ``EXACT``, ``NOT_EXACT``, ``IEXACT``,
        ``NOT_IEXACT``, ``CONTAINS``, ``NOT_CONTAINS``, ``ICONTAINS``, ``NOT_ICONTAINS``,
        ``STARTSWITH``, ``ISTARTSWITH``, ``ENDSWITH``, ``IENDSWITH``, ``REGEX``,
        ``IREGEX``, ``GLOB``, ``NOT_GLOB``, ``IGLOB``, ``NOT_IGLOB``

        **For version-based filters** (``evr``, ``epoch``, ``version``, ``release``):

        - ``"EXACT"`` - equal to
        - ``"NOT_EXACT"`` - not equal to
        - ``"GT"`` - greater than
        - ``"GTE"`` - greater than or equal to
        - ``"LT"`` - less than
        - ``"LTE"`` - less than or equal to

        .. NOTE::
           Version-based filters use proper RPM version comparison semantics
           instead of lexical string comparison. This ensures correct version
           ordering (e.g., "1.10" > "1.9").

        .. NOTE::
           The ``cmdline_repo`` filter only supports ``"EXACT"`` comparator.

``exclude``
    Boolean

    Optional field.

    If ``true``, packages matching the filters are excluded from the rule. This is
    useful for defining exceptions. Exclusion rules must appear before the rules
    from which packages should be excluded.

    Default: ``false``

Examples
========

Example 1: Allow change from "VendorA" to "VendorB"
---------------------------------------------------

This example shows the minimal required configuration, allowing a change from
"VendorA" to "VendorB", but not the reverse.

.. code-block:: toml

    version = '1.1'

    [[outgoing_vendors]]
    vendor = 'VendorA'

    [[incoming_vendors]]
    vendor = 'VendorB'

Example 2: Allow change from any vendor to "My Trusted Vendor"
--------------------------------------------------------------

This example shows allowing a change from any vendor to "My Trusted Vendor",
but not the reverse. Missing ``outgoing_vendors`` means any vendor is allowed.

.. code-block:: toml

    version = '1.1'

    [[incoming_vendors]]
    vendor = 'My Trusted Vendor'

Example 3: Equivalent vendors
-----------------------------

This example shows vendors that are mutually equivalent, allowing changes
in both directions.

.. code-block:: toml

    version = '1.1'

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

    version = '1.1'

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

Example 5: Combining equivalent vendors with incoming vendors
-------------------------------------------------------------

This example demonstrates combining ``equivalent_vendors`` with ``incoming_vendors``,
a feature introduced in version ``"1.1"``. "First Vendor" and "Second Vendor" are mutually
equivalent, and both can change to "Third Vendor".

.. code-block:: toml

    version = '1.1'

    [[equivalent_vendors]]
    vendor = 'First Vendor'

    [[equivalent_vendors]]
    vendor = 'Second Vendor'

    [[incoming_vendors]]
    vendor = 'Third Vendor'

Example 6: Allow command-line packages from any vendor
------------------------------------------------------

This example allows installing packages from the command-line repository
from any vendor, bypassing vendor restrictions.

.. code-block:: toml

    version = '1.1'

    [[incoming_packages]]
    filters = [
      { filter = 'cmdline_repo', value = 'true' }
    ]

Example 7: Command-line packages with exclusion
-----------------------------------------------

This example allows installing packages from the command-line repository
from any vendor, except for packages whose names start with "mypackage".

.. code-block:: toml

    version = '1.1'

    [[incoming_packages]]
    filters = [
      { filter = 'name', value = 'mypackage', comparator = 'STARTSWITH' }
    ]
    exclude = true

    [[incoming_packages]]
    filters = [
      { filter = 'cmdline_repo', value = 'true' }
    ]

Example 8: Allow change from any vendor to specific one with package filtering
------------------------------------------------------------------------------

This example allows a change from any vendor to "My Vendor", but
only for incoming packages whose source package name is "mypackage" and
version is greater than or equal to "2.0".

.. code-block:: toml

    version = '1.1'

    [[incoming_packages]]
    filters = [
      { filter = 'source_name', value = 'mypackage' },
      { filter = 'version', value = '2.0', comparator = 'GTE' }
    ]

    [[incoming_vendors]]
    vendor = 'My Vendor'

See Also
========

* :ref:`DNF5 Vendor Change Policy File Reference <dnf5_vendor_change_policy-label>`
* :ref:`DNF5 Vendor Change Policy File Reference - v1.0 <dnf5_vendor_change_policy_v1_0-label>`
* :manpage:`dnf5.conf(5)`, :ref:`DNF5 Configuration Reference <dnf5_conf-label>`
