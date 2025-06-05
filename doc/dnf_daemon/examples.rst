#####################
 Example D-Bus usage
#####################

Examples using dnf5daemon server via D-Bus from Python.

Print upgrades
==============

Print all available upgrades, the repository they come from and severity of associated advisory if present.

.. literalinclude:: examples/print_upgrades_with_severities.py
    :language: Python
    :linenos:
    :lines: 18-

System upgrade
==============

Perform a system-upgrade.

.. literalinclude:: examples/system_upgrade.py
    :language: Python
    :linenos:
    :lines: 22-

`list_fd()`
===========

:doc:`dnf5daemon_dbus_api.8` `org.rpm.dnf.v0.rpm.Rpm.list_fd()` exmaple.

.. literalinclude:: examples/system_upgrade.py
    :language: Python
    :linenos:
    :lines: 22-
