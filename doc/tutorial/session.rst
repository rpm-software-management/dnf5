.. _creating and configuring a session:

Creating and configuring a session
==================================


.. literalinclude:: tests/session/create_base.cpp
    :language: c++
    :linenos:
    :lines: 2,4-


.. _tutorial-session-force-arch-label:

Override the system architecture
--------------------------------

For the `dnf5` command-line tool, the
:ref:`--forcearch <forcearch_misc_ref-label>`, :manpage:`dnf5-forcearch(7)`
option is available. Here's how you can achieve the same effect using the API:


.. literalinclude:: tests/session/force_arch.cpp
    :language: c++
    :linenos:
    :lines: 2,4-
