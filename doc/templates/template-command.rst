.. _command template:

DNF5 Command Template
=====================

This page focuses on how to write a command with two options in dnf5.

.. note::

    This code is thought to be self explanatory. You should be able to copy
    the snippets, following the directory structure and naming shown above
    each one.


The command header
------------------

.. literalinclude:: tests/dnf5-plugin/template_cmd.hpp
    :language: c++
    :linenos:

The command source
------------------

.. literalinclude:: tests/dnf5-plugin/template_cmd.cpp
    :language: c++
    :linenos:
    :lines: 3-

The argument class(es)
----------------------

.. literalinclude:: tests/dnf5-plugin/arguments.hpp
    :language: c++
    :linenos:

Direct integration into dnf5 codebase
-------------------------------------

.. CAUTION::

    If you are writing an external command to be included in a dnf5
    plugin, **STOP** here and move on to the :ref:`dnf5 plugin template`.
    The remainder of this page is only applicable when writing commands
    to be included directly in the dnf5 codebase (in the ``dnf5/commands/``
    subdirectory).

The command must be included and registered in ``dnf5/main.cpp``

.. code-block:: cpp

    // new commands must be included in main.cpp
    #include "commands/template/template.hpp"

    // commands are registered in the add_commands() function
    context.add_and_initialize_command(
        std::make_unique<TemplateCommand>(context));

Following this example you should have an output like this.

.. code-block::

    $ dnf5 --help
    ...
    Software Management Commands:
      install                                Install software
      upgrade                                Upgrade software
      ...
      template                               A command that prints its name and arguments'
                                             name

.. code-block::

    $ dnf5 template --help
    Usage:
      dnf5 [GLOBAL OPTIONS] template [OPTIONS]

    Options:
      --bar                         print bar
      --foo                         print foo
