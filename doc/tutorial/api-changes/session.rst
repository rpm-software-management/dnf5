Changes in setting up the Base
==============================

Both DNF4 and DNF5 have a class Base, which is the central point of functionality.

In DNF5, a ``libdnf5::Base::setup`` method must be called after configuration and variables are updated, but before loading repositories and it cannot be called multiple times. It loads libdnf plugins, resolves configuration of protected packages and also loads variables, so it replaces also the ``dnf.conf.substitutions.Substitutions.update_from_etc`` method.

DNF4 Python:

.. code-block:: python
  :linenos:

  import dnf

  # Create a new Base object.
  base = dnf.Base()

  # Optionally, load configuration from the file defined in the current
  # configuration.
  base.conf.read()

  # Optionally, load variables.
  base.conf.substitutions.update_from_etc("/")

DNF5 Python:

.. literalinclude:: ../tests/bindings/python3/api_changes_from_dnf4/create_base.py
    :language: py
    :linenos:
    :lines: -5,14-

DNF5 C++:

.. literalinclude:: ../tests/api_changes_from_dnf4/create_base.cpp
    :language: c++
    :linenos:
    :lines: 2,4-7,16-

Setting of configuration and variables should be done before calling the ``libdnf5::Base::setup`` in DNF5.

In Python, there are shortcuts for getting or setting the configuration options using the configuration class attributes (e.g. instead of ``get_skip_unavailable_option().get_value()`` and ``get_skip_unavailable_option().set()`` there is a property ``skip_unavailable``. To access all methods from the option, use standard getters as in C++ API.

DNF5 Python:

.. literalinclude:: ../tests/bindings/python3/api_changes_from_dnf4/configure_base.py
    :language: py
    :linenos:

DNF5 C++:

.. literalinclude:: ../tests/api_changes_from_dnf4/configure_base.cpp
    :language: c++
    :linenos:
