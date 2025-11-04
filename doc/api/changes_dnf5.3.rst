###############################################
 Modifications to the public API in dnf 5.3.0.0
###############################################

This page lists the differences in the public API of `DNF5 5.3 <https://github.com/rpm-software-management/dnf5/releases/tag/5.3.0.0>`_ compared to the previous major version, `DNF5 5.2 <https://github.com/rpm-software-management/dnf5/releases/tag/5.2.0.0>`_.

ABI change of libdnf5-cli
=========================

New version is ``libdnf5-cli.so.3`` and users of libdnf5-cli will need to be recompiled.


Advisory command json output
============================

* Changed timestamps from ISO strings to UNIX integers.
* ``advisory info`` subcommand previously outputted a dictionary where each advisory json was indexed by its name, this was changed to an array.
