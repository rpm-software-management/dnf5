.. _specs_misc_ref-label:

#########################
yum/dnf Repository Format
#########################

There are now a good handful of package managers that have used and evolved
a repository format that is generally referred to as a `yum` repository. The
original release of `yum`, the "Yellowdog Updater Modified", was in 2002,
and was present in Fedora Core 1, released in 2003.

This documentation has the goal of documenting what modern tooling should
generate for `dnf5`, but also cover the quirks of in-the-wild repositories.

Since the repository format is primarily XML based, XML Schema Definition (XSD)
files will be used so that validation can be performed.

Tools parsing the format should accept the permissive variants of the schema
definitions, while tools creating the format should produce the strict variants.

History
=======

There have been previous attempts to document the repository format, most
notably in openSUSE.
https://en.opensuse.org/openSUSE:Standards_Rpm_Metadata

There exist repositories using a SQLite variant of the repository format,
with some repositories *exclusively* using it, having only the `repomd.xml`
top level file in XML. The SQLite variant is understood by `yum`, but did
not make the leap to `dnf` at all.

The SQLite repository format variant is not understood by `libsolv`, which is
used by most current generation RPM package managers.

Repository Structure
====================

The basic structure is that a `repodata/repomd.xml` file exists, which points
to all other metadata about the repository.
