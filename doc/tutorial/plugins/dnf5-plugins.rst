.. _dnf5 plugins tutorial:

DNF5 Plugins
============

This is the way for adding a new command to DNF5, allowing users to simply type
``dnf5 new-command`` and access the implemented functionality.

Command vs Plugin
-----------------

The reason why you can't directly implement a new DNF5 command is the following.

The core commands in ``dnf5/commands`` implement essential package manager
functionality, primarily focused on installing and managing packages. They are
integral to the DNF5 package.

Plugin commands implement additional features that extend the capabilities
of DNF5. They are packaged separately for optional installation by users who
need these additional features.

Thus, core commands and plugins differ by logic and implementation.

Writing an Active Plugin
------------------------

A DNF5 plugin comprises one or more commands. Visit the :ref:`command template`
to see how to implement a command.

Note that for plugin purposes, we don't want to register new commands
in the ``dnf5/main.cpp`` file. Instead, we will implement the ``dnf5::IPlugin``
interface by reusing the existing boilerplate code, as shown below (refer to the
comments in the provided code for the expected input locations):

.. literalinclude:: ../../templates/tests/dnf5-plugin/template_cmd_plugin.cpp
    :language: c++
    :linenos:

.. note::
    During the startup of the DNF5 application, the plugin library scans
    the files in the configured plugins directory to check for the presence
    of any plugins implementing the common ``dnf5::IPlugin`` interface.
    If such plugins are found, they are loaded into memory along with all
    the commands they implement.

Each plugin source is structured in its own directory within the
``dnf5-plugins`` folder. See other plugins, such as ``builddep_plugin``:

.. code-block:: bash

    $ tree dnf5-plugins/builddep_plugin/
    dnf5-plugins/builddep_plugin/
    ├── builddep_cmd_plugin.cpp
    ├── builddep.cpp
    ├── builddep.hpp
    └── CMakeLists.txt

Building the Binary
-------------------

To create the plugin binary, add a CMake build script:

.. literalinclude:: ../../templates/tests/dnf5-plugin/CMakeLists.txt
    :language: cmake
    :linenos:

Include the newly created plugin in the ``CMakeLists.txt`` parent file inside
``dnf5-plugins`` like this: ``add_subdirectory("template_plugin")``.

Delivering to the User
----------------------

We still need to set up the deployment process so that users can install
the new plugin into DNF5 through the package manager.

Add a new provide in the ``dnf5-plugins`` section of the ``dnf5.spec`` file:
``Provides: dnf5-command(template)``.

You should also include a documentation man page describing the usage of your
plugin in detail. There are several places which need to be interconnected,
make sure to complete all the following steps:

* ``doc/dnf5_plugins/template.8.rst``: Add a new man page for your plugin with the respective name.
* ``doc/dnf5_plugins/index.rst``: Reference the new plugin in the DNF5 plugins page.
* ``doc/dnf5.8.rst``: Reference the new plugin in the main DNF5 man page.
* ``doc/CMakeLists.txt`` and ``doc/conf.py.in``: Integrate with Sphinx documentation.
* ``dnf5.spec``: Include the new man page in the ``dnf5-plugins`` package.
