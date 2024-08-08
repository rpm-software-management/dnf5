Tutorial: Writing Plugins
=========================

Plugins are the means to extend the existing functionality of DNF5. They are
written in the native language of DNF5, which is C++. Two types of plugins are
supported:

* :ref:`dnf5 plugins`:
    * **Active:** Used to implement one or more commands.
* :ref:`libdnf5 plugins`:
    * **Passive:** Used to implement additional logic into the library using hooks.

.. note::
    Existing plugins from the preceding DNF project are not compatible
    with the new DNF5. A different API is now used, and they were
    written in Python, which is not a mandatory component in DNF5.

For detailed information on both types of plugins, refer to their respective pages:

.. toctree::
    :maxdepth: 2

    dnf5-plugins
    libdnf5-plugins

Debugging Tips
--------------

To test your freshly built DNF5 Plugin, redirect DNF5 to load it by setting
the ``DNF5_PLUGINS_DIR`` environmental variable to your build directory (e.g.,
``DNF5_PLUGINS_DIR=~/dnf5/build/dnf5-plugins/template_plugin``).

Speaking about LIBDNF5 Plugins, utilize the ``LIBDNF_PLUGINS_CONFIG_DIR``
environmental variable to configure the directory with the plugin's configuration.
This can also be overridden by the ``pluginconfpath`` configuration option.
Additionally, set the directory for the plugin binaries with the ``pluginpath``
configuration option.

Ensure effective debugging by building the project with debugging symbols
(``-DCMAKE_BUILD_TYPE=Debug``).
