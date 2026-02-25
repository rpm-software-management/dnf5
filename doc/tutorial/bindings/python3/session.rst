.. _bindings python3 creating and configuring a session:

Creating and configuring a session
==================================


.. literalinclude:: ../../tests/bindings/python3/session/create_base.py
    :language: python
    :linenos:


.. _tutorial-bindings-python3-session-force-arch-label:

Override the system architecture
--------------------------------

For the `dnf5` command-line tool, the
:ref:`--forcearch <forcearch_misc_ref-label>`, :manpage:`dnf5-forcearch(7)`
option is available. Here's how you can achieve the same effect using the Python API:

.. literalinclude:: ../../tests/bindings/python3/session/force_arch.py
    :language: python
    :linenos:
    :lines: -8,13-
