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

    Supported values: ``"1.0"``, ``"1.1"``

    This field is mandatory and must be specified at the top level of the TOML file.

    Example::

        version = '1.1'

Vendor Mapping Definition
--------------------------

The file can use one or more of the following vendor definition methods.
For version ``"1.0"``, only one method can be used. Version ``"1.1"`` allows
combining ``equivalent_vendors`` with ``outgoing_vendors`` and ``incoming_vendors``.

**Option 1: Separate Lists (outgoing + incoming)**

``[[outgoing_vendors]]``
    Array of tables

    List of vendors from which changes are allowed.

``[[incoming_vendors]]``
    Array of tables

    List of vendors to which changes are allowed.

    .. NOTE::
       For version ``"1.0"``: ``outgoing_vendors`` and ``incoming_vendors`` must be either both present or both missing.

       For version ``"1.1"``: Either array can be independently present or missing. A missing vendors array means any vendor is allowed.

**Option 2: Equivalent Vendors**

``[[equivalent_vendors]]``
    Array of tables

    List of vendors that are mutually equivalent. Changes are allowed in both directions
    between all vendors in this list.

    .. NOTE::
       For version ``"1.0"``: Cannot combine ``equivalent_vendors`` with ``outgoing_vendors`` or ``incoming_vendors``.
       However, ``equivalent_vendors`` can be replaced by specifying the same list of vendors
       in both ``outgoing_vendors`` and ``incoming_vendors``.

       For version ``"1.1"``: Can be freely combined with ``outgoing_vendors`` and ``incoming_vendors``.

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

For version ``"1.1"``, vendor change policies can be further restricted to apply only
to specific packages using package filters. These are optional and allow fine-grained
control over which packages are allowed to change vendors.

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
        - ``"arch"`` - package architecture
        - ``"repoid"`` - repository ID
        - ``"cmdline_repo"`` - whether the package is from command-line repository (boolean)

    ``value``
        String

        Required field.

        The value to match against. For ``cmdline_repo`` filter, use ``"true"``/``"1"``
        or ``"false"``/``"0"``.

    ``comparator``
        String

        Optional field.

        The matching method to use. Same values as for vendor comparator (see above).

        Default: ``"EXACT"``

        .. NOTE::
           The ``cmdline_repo`` filter only supports ``"EXACT"`` comparator.

``exclude``
    Boolean

    Optional field.

    If ``true``, packages matching the filters are excluded from the rule. This is
    useful for defining exceptions. Exclusion rules must appear before the rules
    from which packages should be excluded.

    Default: ``false``

Error Conditions
================

The following configurations are **invalid** and will cause an error:

- Missing ``version`` field
- Unsupported version (other than ``"1.0"`` or ``"1.1"``)
- For version ``"1.0"`` only:

  - Combination of ``equivalent_vendors`` with ``outgoing_vendors`` or ``incoming_vendors``
  - Only ``outgoing_vendors`` without ``incoming_vendors`` (or vice versa)

- Missing required ``vendor`` field in a vendor entry
- Missing required ``filter`` or ``value`` field in a package filter
- Missing ``filters`` array in a package entry
- Empty ``filters`` array in a package entry
- Unknown ``filter`` value (must be one of: "name", "source_name", "arch", "repoid", "cmdline_repo")
- Unknown ``comparator`` value
- Invalid ``comparator`` for ``cmdline_repo`` filter (only EXACT is supported)
- Invalid ``value`` for ``cmdline_repo`` filter (must be "true"/"1" or "false"/"0")
- Invalid regex pattern in filter value
- Unknown keys at the top level or inside entries

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
but not the reverse.

**Version "1.0"** (requires explicit rule for allowing all vendors):

.. code-block:: toml

    version = '1.0'

    [[outgoing_vendors]]
    vendor = ''
    comparator = 'CONTAINS'

    [[incoming_vendors]]
    vendor = 'My Trusted Vendor'

**Version "1.1"** (missing ``outgoing_vendors`` means any vendor):

.. code-block:: toml

    version = '1.1'

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

Example 4: Combining equivalent vendors with incoming vendors
-------------------------------------------------------------

This example demonstrates combining ``equivalent_vendors`` with ``incoming_vendors``,
a feature introduced in version ``"1.1"``. "First Vendor" and "Second Vendor" are mutually
equivalent, and both can change to "Third Vendor".

**Version "1.0"** (cannot combine ``equivalent_vendors`` with ``incoming_vendors``):

.. code-block:: toml
    version = '1.0'

    [[outgoing_vendors]]
    vendor = 'First Vendor'

    [[outgoing_vendors]]
    vendor = 'Second Vendor'

    [[incoming_vendors]]
    vendor = 'First Vendor'

    [[incoming_vendors]]
    vendor = 'Second Vendor'

    [[incoming_vendors]]
    vendor = 'Third Vendor'

**Version "1.1"** (can combine ``equivalent_vendors`` with ``incoming_vendors`` and ``outgoing_vendors``):

.. code-block:: toml
    version = '1.1'

    [[equivalent_vendors]]
    vendor = 'First Vendor'

    [[equivalent_vendors]]
    vendor = 'Second Vendor'

    [[incoming_vendors]]
    vendor = 'Third Vendor'

Example 5: Equivalent vendors with an exclusion
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

This example allows a change from any vendor to "My Trusted Vendor", but
only for incoming packages whose source package name is "mypackage" and
are located in the 'myrepo' repository.

.. code-block:: toml

    version = '1.1'

    [[incoming_packages]]
    filters = [
      { filter = 'source_name', value = 'mypackage' },
      { filter = 'repoid', value = 'myrepo' }
    ]

    [[incoming_vendors]]
    vendor = 'My Trusted Vendor'

See Also
========

* :manpage:`dnf5.conf(5)`, :ref:`DNF5 Configuration Reference <dnf5_conf-label>`
