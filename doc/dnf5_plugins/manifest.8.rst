..
    Copyright Contributors to the DNF5 project.
    SPDX-License-Identifier: GPL-2.0-or-later
    Adapted from documentation for the DNF4 dnf-plugins-core manifest plugin

.. _manifest_plugin_ref-label:

##################
 Manifest Command
##################

Synopsis
========

``dnf5 manifest new [options] [<package-spec-NPFB>...]``

``dnf5 manifest download [options]``

``dnf5 manifest install [options]``


Description
===========

For working with RPM package manifest files using the `libpkgmanifest <https://github.com/rpm-software-management/libpkgmanifest>`_ library.

:ref:`skip_if_unavailable <_skip_if_unavailable_options-label>` and `skip_broken <_skip_broken_options-label` will be set to ``false`` for all operations.


``new``
    Generate a manifest file based on the provided input.

    Supported input forms include:
    - Package specifications
    - Prototype input file
    - None (defaults to using installed packages)

    The input is resolved, and all relevant packages and dependencies
    are saved in the manifest file.

    See options and examples for more details.

    Without specific input, the manifest is generated solely from
    installed packages.

``download``
    Download all packages specified in the manifest file to disk.

    By default, packages are downloaded to a subfolder named after the
    manifest file. You can also use the ``--destdir`` option to
    specify a custom directory for the downloaded packages.

    The manifest file may contain packages for multiple base architectures.
    By default, only packages corresponding to the current system architecture
    are downloaded. To download packages for specific architectures, use the
    ``--archs`` option.

``install``
    Install all packages specified in the manifest file.

---------
Arguments
---------

``<package-spec-NPFB>``
    Specification for including a package in the manifest file.
    Local RPMs are not supported.
    For more information, refer to :ref:`Specifying Packages <specifying_packages-label>`.

-------
Options
-------

``--input``
    Specify a custom path for the prototype input file.
    By default, ``rpms.in.yaml`` is used.

``--manifest``
    Specify a custom path for the manifest file.
    By default, ``packages.manifest.yaml`` is used.

``--source``
    Include source packages in consideration.
    Not supported for the ``install`` command.

``--use-system``
    Consider currently installed system packages for dependency resolution
    and exclude them from the resulting manifest file.

``--archs``
    Specify a comma-separated list of architectures to work with.
    By default, only packages corresponding to the current system architecture are processed.

``--per-arch``
    Create a separate manifest file for each requested base architecture.
    By default, a single manifest file is created for all architectures.

--------
Examples
--------

``dnf manifest new``
    If the ``rpms.in.yaml`` prototype input file is in the current directory, it will be used
    for package dependency resolution. If not, the manifest will be generated from the
    system's installed packages.

    The new manifest file is created at the default location: ``packages.manifest.yaml``.

``dnf manifest new alsa-lib alsa-tools``
    Create a new manifest file containing the ``alsa-lib`` and ``alsa-tools`` packages along
    with all their dependencies.

``dnf manifest new wget --use-system``
    Create a new manifest file including the ``wget`` package and all its uninstalled dependencies.

``dnf manifest download --manifest /home/user/Downloads/manifest.yaml --source``
    Download all packages, including source packages, specified in the given manifest file.

``dnf manifest install -y``
    Install all packages specified in the manifest file located in the current directory
    under the default file name, automatically answering "yes" to all prompts during the
    transaction resolution.

--------
See Also
--------

* `libpkgmanifest upstream <https://github.com/rpm-software-management/libpkgmanifest>`_
* `Prototype input file specification <https://github.com/konflux-ci/rpm-lockfile-prototype?tab=readme-ov-file#whats-the-input_file>`_
