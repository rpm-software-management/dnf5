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

..
    # TODO(jkolarik): Add more specs info (advisory-spec, repo-spec, ...)

.. _specs_misc_ref-label:

#######################
 Patterns Specification
#######################

Description
===========

Different pattern matching rules for arguments in ``*-spec`` form are defined
for operation with each entity type in ``DNF5``. In this section all rules are
described and explained in detail, with examples.


Globs
=====

Pattern specification supports the same glob pattern matching that shell does.
The following patterns are supported:

``*``
    Matches any number of characters.
``?``
    Matches any single character.
``[]``
    Matches any one of the enclosed characters. A pair of characters separated
    by a hyphen denotes a range expression; any character that falls between
    those two characters, inclusive, is matched. If the first character
    following the ``[`` is a ``!`` or a ``^`` then any character not enclosed
    is matched.

Note: Curly brackets (``{}``) are not supported. You can still use them in
shells that support them and let the shell do the expansion, but if quoted or
escaped, ``DNF5`` will not expand them.

.. _nevra_matching_ref-label:

NEVRA matching
==============

Each package can be uniquely identified by the `NEVRA` string. It consists of
5 parts of information:

`Name`
    | Package name.

`Epoch`
    | Epoch number.
    | It is not always included.

    The epoch number overrides other version checking, so it can be used to
    force the package upgrade over some other one.

`Version`
    | Version string.
    | It is not strictly numeric.
    | It is intended to match the upstream software version.

`Release`
    | Edition string.

    It is an information about the particular package build, usually a number
    increased with the newer build. It is not connected with the upstream software.

`Architecture`
    | Target architecture string.
    | Defines the processor type the package is intended to be installed on.

    It can be also a package containing source files (``src``) or architecture-independent
    package (``noarch``).

When matching against NEVRAs, partial matching is supported. ``DNF5`` tries to match
the spec against the following list of NEVRA forms (in decreasing order of
priority):

    |
    | ``NAME-[EPOCH:]VERSION-RELEASE.ARCH``
    | ``NAME.ARCH``
    | ``NAME``
    | ``NAME-[EPOCH:]VERSION-RELEASE``
    | ``NAME-[EPOCH:]VERSION``
    |

Note that `name` can in general contain dashes (e.g. ``package-with-dashes``).

The first form that matches any packages is used and the remaining forms are
not tried. If none of the forms match any packages, an attempt is made to match
the ``<package-spec>`` against full package NEVRAs. This is only relevant
if globs are present in the ``<package-spec>``.

``<package-spec>`` matches NEVRAs the same way ``<package-name-spec>`` does,
but in case matching NEVRAs fails, it attempts to match against provides and
file provides of packages as well.

You can specify globs as part of any of the five NEVRA components. You can also
specify a glob pattern to match over multiple NEVRA components (in other words,
to match across the NEVRA separators). In that case, however, you need to write
the spec to match against full package NEVRAs, as it is not possible to split
such spec into NEVRA forms.


Packages
========

Many commands take a ``<package-spec>`` parameter that selects a package for
the operation. The ``<package-spec>`` argument is matched against package
NEVRAs, provides and file provides.

When ``<package-spec>`` is a package name or a provide, the user can provide
additional restriction rules for matching the arguments. Basic version comparisons
can be used for this purpose (=, >, <, >=, <=), like this ``<package-spec> >= <version>``,
where the ``<version>`` argument is in a ``[EPOCH:]VERSION[-RELEASE]`` format
as specified above in the :ref:`NEVRA matching <nevra_matching_ref-label>` section.

To build more complex expressions, a rich dependency feature
is also supported, which is always enclosed in parentheses. Boolean
operators and nesting can be used, f.e. ``(<spec1> or (<spec2> and <spec3>))``.
For more information, please see :ref:`RPM boolean dependencies <rpm_bool_deps_ref-label>`.

``<package-file-spec>`` is similar to ``<package-spec>``, except provides
matching is not performed. Therefore, ``<package-file-spec>`` is matched only
against NEVRAs and file provides.

``<package-name-spec>`` is matched against NEVRAs only.


Provides
========

``<provide-spec>`` in command descriptions means the command operates on
packages providing the given spec. This can either be an explicit provide, an
implicit provide (i.e. name of the package) or a file provide. The selection is
case-sensitive and globbing is supported.

.. _file_provides_ref-label:

Specifying File Provides
------------------------

If a ``spec`` starts with either ``/`` or ``*/``, it is considered as a potential file provide.


Groups
======

``<group-spec>`` allows one to select (environment) groups a particular operation should work
on. It is a case insensitive string (supporting globbing characters) that is
matched against a group's ID, canonical name and name translated into the
current ``LC_MESSAGES`` locale (if possible).


Modules
=======

``<module-spec>`` allows one to select modules or profiles a particular operation should work
on.

Since `NEVRA` matching form is insufficient for modules, they are uniquely identified by the
`NSVCA` format (``NAME:STREAM:VERSION:CONTEXT:ARCH/PROFILE``). Supported partial forms are the following:

    |
    | ``NAME``
    | ``NAME:STREAM``
    | ``NAME:STREAM:VERSION``
    | ``NAME:STREAM:VERSION:CONTEXT``
    | All above combinations with ``::ARCH`` (e.g. ``NAME::ARCH``)
    | ``NAME:STREAM:VERSION:CONTEXT:ARCH``
    | All above combinations with ``/PROFILE`` (e.g. ``NAME/PROFILE``)
    |

In case stream is not specified, the enabled or the default stream is used, in this order.
In case profile is not specified, the system default profile or the 'default' profile is used.


Transactions
============

``<transaction-spec>`` can be in one of several forms. If it is an integer, it
specifies a transaction ID. Specifying ``last`` is the same as specifying the ID
of the most recent transaction. The last form is ``last-<offset>``, where
``<offset>`` is a positive integer. It specifies offset-th transaction preceding
the most recent transaction.


See Also
========

.. _rpm_bool_deps_ref-label:

RPM boolean dependencies:
    | https://rpm-software-management.github.io/rpm/manual/boolean_dependencies.html
