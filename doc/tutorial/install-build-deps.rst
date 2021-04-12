Installing Build Requirements
=============================


Before we can start using libdnf, we need the header files or runtime modules installed.
On Fedora, we run one of the following commands according to our needs.

Enable COPR repo with the DNF 5 nightly builds::

    $ dnf copr enable rpmsoftwaremanagement/dnf5-unstable

C++::

    $ dnf install libdnf5-devel libdnf-cli-devel


Python 3::

    $ dnf install python3-libdnf5 python3-libdnf-cli-devel


Perl 5::

    $ dnf install perl5-libdnf perl5-libdnf-cli


Ruby::

    $ dnf install perl5-libdnf perl5-libdnf-cli


.. note::
    While the libdnf packages are always needed, we can omit installing the libdnf-cli
    packages if we're not using any of the command-line features they implement.


.. warning::
    The libdnf5-devel and python3-libdnf5 packages will be renamed once they don't conflict
    with the packages from the currently used DNF 4 stack.
