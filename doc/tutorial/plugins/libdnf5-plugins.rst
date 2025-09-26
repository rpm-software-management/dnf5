.. _libdnf5 plugins tutorial:

LIBDNF5 Plugins
===============

These plugins enable changes to the existing DNF5 workflow.
Possible uses of DNF5 passive plugins are modifying the LIBDNF5's
behavior at specific breakpoints, changing or implementing the logic,
or triggering the loading of additional plugins via prepared callbacks.

Writing a Passive Plugin
------------------------

Similarly to the :ref:`dnf5 plugins tutorial`, we have to implement
the ``libdnf5::plugin::IPlugin`` interface and override the hooks
to alter DNF5's logic.

In the following code block, a simple example plugin introduces logic in two
different steps. The first is after preparing the ``libdnf5::Base`` object.
The second is before the start of the transaction.

.. literalinclude:: ../../templates/libdnf5-plugin/template.cpp
    :language: c++
    :linenos:

Each plugin is structured in its own directory within the ``libdnf5-plugins``
folder. Review other plugins, such as ``actions``, to understand the expected
structure:

.. code-block:: bash

    $ tree libdnf5-plugins/actions/
    libdnf5-plugins/actions/
    ├── actions.conf
    ├── actions.cpp
    └── CMakeLists.txt

Building the Binary
-------------------

To create the plugin binary, include a CMake build script:

.. literalinclude:: ../../templates/libdnf5-plugin/CMakeLists.txt
    :language: cmake
    :linenos:

Unlike the :ref:`dnf5 plugins tutorial`, plugins are part of the same domain
as the core DNF5 functionality. Individual plugins are optionally included
in the binary using created macro expressions in the spec file:

.. code-block:: spec

    # ========== build options ==========
    ...
    %bcond_without plugin_template
    ...
    # ========== unpack, build, check & install ==========
    ...
    %build
    %cmake \
        ...
        -DWITH_PLUGIN_TEMPLATE=%{?with_plugin_template:ON}%{!?with_plugin_template:OFF} \
        ...

Define the connected CMake option in the ``dnf5/CMakeLists.txt`` file:

.. code-block:: cmake

    option(WITH_PLUGIN_TEMPLATE "Build a DNF5 template plugin" ON)

This sets up the default behavior to include the plugin in the build.

Include the newly created plugin in the ``CMakeLists.txt`` parent file inside
``libdnf5-plugins``: ``add_subdirectory("template")``.

Delivering to the User
----------------------

Each plugin requires a mandatory configuration file where we need to define the plugin's name
and specify when to enable it during runtime. Options include:

* ``no``: Plugin is disabled.
* ``yes``: Plugin is enabled.
* ``host-only``: Plugin is enabled only in configurations without installroot.
* ``installroot-only``: Plugin is enabled only in configurations with installroot.

Additional optional configuration options and sections can be defined and then accessed from
the plugin implementation. Here's an example configuration file:

.. literalinclude:: ../../templates/libdnf5-plugin/template.conf
    :language: cfg
    :linenos:

In the :ref:`libdnf5 plugins tutorial`, each plugin is delivered in a separate
package. Include a new section in the spec file for this purpose:

.. code-block:: spec

    # ========== libdnf5-plugin-template ==========

    %if %{with plugin_template}
    %package -n libdnf5-plugin-template
    Summary:        Libdnf5 template plugin
    License:        LGPL-2.1-or-later
    Requires:       libdnf5%{?_isa} = %{version}-%{release}

    %description -n libdnf5-plugin-template
    Include a more descriptive message about your plugin here.

    %files -n libdnf5-plugin-template
    %{_libdir}/libdnf5/plugins/template.*
    %config %{_sysconfdir}/dnf/libdnf5-plugins/template.conf
    %endif

Consider including a documentation man page describing your plugin's functionality.
Ensure that you complete all the following steps:

* ``doc/libdnf5_plugins/template.8.rst``: Add a new man page for your plugin with the respective name.
* ``doc/libdnf5_plugins/index.rst``: Add a reference to the new plugin to the LIBDNF5 plugins page.
* ``doc/CMakeLists.txt`` and ``doc/conf.py.in``: Integrate with Sphinx documentation.
* ``dnf5.spec``: Include the new man page in your newly created package.
