.. _command template:

DNF5 Command Template
=====================

This page focuses on how to write a command with two options in dnf5.

.. note::

    This code is thought to be self explanatory. You should be able to copy
    the snippets, following the directory structure and naming shown above
    each one.

``dnf5/command/template.hpp``

.. literalinclude:: command/template.hpp
    :language: c++
    :linenos:

``dnf5/command/template.cpp``

.. literalinclude:: command/template.cpp
    :language: c++
    :linenos:

``dnf5/command/arguments.hpp``

.. literalinclude:: command/arguments.hpp
    :language: c++
    :linenos:

The command must be included and registered in ``dnf5/main.cpp``

.. code-block:: cpp

    // new commands must be included in main.cpp
    #include "commands/template/template.hpp"

    // commands must be registered like this
    register_subcommand(std::make_unique<TemplateCommand>(*this), software_management_commands_group);

Following this example you should have an output like this.

.. code-block::

    $ dnf5 --help
    ...
    Software Management Commands:
      install                                Install software
      upgrade                                Upgrade software
      ...
      template                               A command that prints its name and arguments' name

.. code-block::

    $ dnf5 template --help
    Usage:
      dnf5 template [GLOBAL OPTIONS] [OPTIONS] [ARGUMENTS]

    Options:
      --bar                         print bar
      --foo                         print foo
