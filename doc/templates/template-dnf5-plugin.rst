.. _dnf5 plugin template:

DNF5 Plugin Template
====================

Below is a template code for a DNF5 plugin. For the complete tutorial on writing
DNF5 plugins, refer to :ref:`dnf5 plugins tutorial`.

``dnf5-plugins/template_plugin/template_cmd_plugin.cpp``

.. literalinclude:: tests/dnf5-plugin/template_cmd_plugin.cpp
    :language: c++
    :linenos:

``dnf5-plugins/template_plugin/CMakeLists.txt``

.. literalinclude:: tests/dnf5-plugin/CMakeLists.txt
    :language: cmake
    :linenos:
