..
    Copyright Contributors to the libdnf project.
    SPDX-License-Identifier: GPL-2.0-or-later

    This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

    Libdnf is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    Libdnf is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

.. _actions_plugin_ref-label:

###############
 Actions Plugin
###############

Description
===========

This plugin allows defining actions to be executed through libdnf5 callbacks hooks.
Each action is hooked to one specific callback. Actions for ``goal_resolved``, ``pre_transaction`` and
``post_transaction`` callbacks may define a (glob-like) filtering rule on the package
NEVRA or package files, as well as whether the package is incoming or outgoing.


Configuration
=============

The plugin does not extend the standard configuration. However, it reads "actions" files.

The actions files are read from ``<libdnf5_plugins_config_dir>/actions.d/`` directory. Only files
with ".actions" extension are read.


Actions file format
===================

Empty lines and lines that start with a '#' character (comment line) are ignored.

Each non-comment line defines an action and consists of five items separated by colons: ``callback_name:package_filter:direction:options:command``.

``callback_name``

   * ``pre_base_setup``
   * ``post_base_setup``
   * ``repos_configured``  (added in version 1.0.0)
   * ``repos_loaded``  (added in version 1.0.0)
   * ``pre_add_cmdline_packages``  (added in version 1.0.0)
   * ``post_add_cmdline_packages``  (added in version 1.0.0)
   * ``goal_resolved`` (added in version 1.3.0)
   * ``pre_transaction``
   * ``post_transaction``

``package_filter``
   A (glob-like) filtering rule applied on the package NEVRA (also in the shortened forms) or package files.

   Empty filter means executing the command once with no information about the package.
   The "*" filter means executing the command for each package in the transaction that matches the ``direction`` filter.
   The filter can be non-empty only for ``goal_resolved``, ``pre_transaction`` and ``post_transaction`` callbacks.

``direction``
   Filters packages by their direction (coming into the system/going out of the system) in a transaction.
   If empty, the filter is not applied.
   The filter can only be non-empty when combined with a non-empty ``package_filter``.

   * ``in`` - packages coming to the system (downgrade, install, reinstall, upgrade)
   * ``out`` - packages going out of the system (upgraded, downgraded, reinstalled, removed, replaced/obsoleted)

``options``
   Options are separated by spaces. A space within an option can be written using escaping.

   * ``enabled=<value>`` - the value specifies when the action is enabled (added in version 0.3.0)

      * ``1`` - action is always enabled
      * ``host-only`` - the action is only enabled for operations on the host
      * ``installroot-only`` - the action is only enabled for operations in the alternative "installroot"

   * ``raise_error=<value>`` - the <value> specifies how the action process errors are handled. What happens if
     the action process did not start or ended with a non-zero exit code or ended abnormally (received a signal)
     or an error occurred during communication (syntax error, communication interrupt, failed to process the output
     line in plain communication mode). If the option is not present, ``raise_error=0`` for backward compatibility.
     (added in version 1.4.0)

      * ``0`` - the errors are logged
      * ``1`` - an exception is thrown out

``command``
   Any executable file with arguments.

   Arguments are separated by spaces. A space within an argument can be written using escaping.
   Escaping can also be used to prevent substitution and to pass special characters \\a, \\b, \\f, \\n, \\r, \\t, \\v.
   Unescaping of arguments is done after substitution.

   The following variables in the command will be substituted:

   * ``${pid}`` - process ID
   * ``${plugin.version}`` - version of the actions plugin (added in version 0.3.0)
   * ``${conf.<option_name>}`` - option from base configuration
   * ``${conf.<repoid_pattern>.<option_name>[=<value_pattern>]}`` - list of "repoid.option=value" pairs (added in version 1.1.0)
   * ``${var.<variable_name>}`` - variable
   * ``${tmp.<actions_plugin_variable_name>}`` - variable exists only in actions plugin context
   * ``${pkg.<package_attribute_name>}`` - value of the package attribute

      * ``${pkg.name}`` - package name
      * ``${pkg.arch}`` - package arch
      * ``${pkg.version}`` - package version
      * ``${pkg.release}`` - package release
      * ``${pkg.epoch}`` - package epoch
      * ``${pkg.na}`` - package name.architecture
      * ``${pkg.evr}`` - package epoch-version-release
      * ``${pkg.nevra}`` - package nevra (0 epoch is hidden)
      * ``${pkg.full_nevra}`` - package full nevra (epoch is always present) ${name}-${epoch}:${ver}-${rel}.${arch}
      * ``${pkg.repo_id}`` - package repository id
      * ``${pkg.license}`` - package license
      * ``${pkg.location}`` - the change of package state in the transaction:
      * ``${pkg.vendor}`` - package vendor
      * ``${pkg.action}`` - action performed on the package:

         * I - newly installed package
         * U - package installed as an upgrade
         * D - package installed as an downgrade
         * R - package used for reinstallation
         * E - erased (removed) package from the system (was upgraded/downgraded/reinstalled/removed)
         * O - replaced (obsoleted) package

   The command will be evaluated for each package that matched the ``package_filter`` and
   the ``direction``. However, after variable substitution, any duplicate commands will be
   removed and each command will only be executed once per transaction.
   The commands are executed in sequence. There is no parallelism. Argument substitution is performed
   after the previous command has completed. This allows the substitution to use the results of the previous commands.
   The order of execution of the commands follows the order in the action files, but may differ from the order of
   packages in the transaction. In other words, when you define several action lines for the same
   ``package_filter`` and ``direction`` these lines will be executed in the order they were defined in the action
   file when the ``package_filter`` and ``direction`` matches a package. However, the order
   of when a particular ``package_filter`` is invoked depends on the position
   of the corresponding package in the transaction.

   The ``repoid.option=value`` pairs in the list are separated by the ',' character.
   The ',' character in the value is replaced by the escape sequence ``"\x2C"``.
   If ``value_pattern`` is used, only pairs with the matching value are listed.
   The ``repoid_pattern`` and ``value_pattern`` can contain globs.


Action standard output format
=============================

The standard output of each executed action (command) is captured and processed.
Each line of output can change the value of a base configuration option, the value
of a configuration option in matching repositories, or a variable.
It can also set or unset one actions plugin variable. The value of this variable is available
for the following commands using the ``${tmp.<actions_plugin_variable_name>}`` substitution.

Actions should change the repositories configuration in the ``repos_configured`` hook.
At this point, the repositories configuration is loaded but not yet applied.

Since version 1.4.0, the output line can write a message to the logger, throw a stop exception
and an error exception.

Output line format
------------------
* ``tmp.<actions_plugin_variable_name>=<value>`` - sets the value of action plugins variable <actions_plugin_variable_name>

* ``tmp.<actions_plugin_variable_name>`` - removes the action plugins variable if it exists

* ``conf.<option_name>=<value>`` -  sets the value of option <option_name> in the base configuration

* ``conf.<repoid_pattern>.<option_name>=<value>`` -  sets the value of option <option_name> in the matching repositories (added in version 1.1.0)

* ``var.<variable_name>=<value>`` - sets value of the vatiable <variable_name>

* ``stop=<message>`` - throws a stop exception with <message> (added in version 1.4.0)

* ``error=<message>`` - the error <message> is logged or throws error exception with <message> if ``raise_error=1``
  (added in version 1.4.0)

* ``log.<level>=<message>`` - writes <message> to the logger with priority <level> (added in version 1.4.0)

    Levels: ``CRITICAL``, ``ERROR``, ``WARNING``, ``NOTICE``, ``INFO``, ``DEBUG``, ``TRACE``


An example actions file:
========================
.. code-block:: none

   # Prints header with process id
   pre_base_setup::::/usr/bin/sh -c echo\ -------------------------------------\ >>/tmp/actions-trans.log
   pre_base_setup::::/usr/bin/sh -c date\ >>/tmp/actions-trans.log
   pre_base_setup::::/usr/bin/sh -c echo\ libdnf5\ pre_base_setup\ was\ called.\ Process\ ID\ =\ '${pid}'.\ >>/tmp/actions-trans.log
   pre_base_setup:::enabled=installroot-only:/usr/bin/sh -c echo\ run\ in\ alternative\ "installroot":\ installroot\ =\ '${conf.installroot}'\ >>/tmp/actions-trans.log

   # Prints the value of the configuration option "defaultyes".
   pre_base_setup::::/bin/sh -c echo\ 'pre_base_setup:\ conf.defaultyes=${{conf.defaultyes}}'\ >>\ {context.dnf.installroot}/actions.log

   # Prints a message that the "post_base_setup" callback was called.
   post_base_setup::::/usr/bin/sh -c echo\ libdnf5\ post_base_setup\ was\ called.\ >>/tmp/actions-trans.log

   # Prints a list of configured repositories with their enable state.
   repos_configured::::/usr/bin/sh -c echo\ Repositories:\ ${conf.*.enabled}\ >>/tmp/repos.log

   # Prints a list of repositories that use the http protocol in baseurl.
   repos_configured::::/usr/bin/sh -c echo\ "${conf.*.baseurl=*http://*}"\ >>/tmp/baseurl_http.log

   # Disables all repositories whose id starts with "rpmfusion".
   repos_configured::::/usr/bin/sh -c echo\ conf.rpmfusion*.enabled=0

   # Prints the information about the start of the transaction.
   # Since package_filter is empty, it executes the commands once.
   pre_transaction::::/usr/bin/sh -c echo\ Transaction\ start.\ Packages\ in\ transaction:\ >>/tmp/actions-trans.log

   # Logs all packages (package action, full_nevra, repo id) in transaction into a file.
   # Uses the shell command "echo" and redirection to a file.
   pre_transaction:*:::/usr/bin/sh -c echo\ '${pkg.action}'\ '${pkg.full_nevra}'\ '${pkg.repo_id}'\ >>/tmp/actions-trans.log

   # Prints the date and time and information about the end of the transaction.
   # Since package_filter is empty, it executes the commands once.
   post_transaction::::/usr/bin/sh -c date\ >>/tmp/actions-trans.log
   post_transaction::::/usr/bin/sh -c echo\ Transaction\ end.\ Repositories\ used\ in\ the\ transaction:\ >>/tmp/actions-trans.log

   # Logs all the repositories from which packages were used in the transaction to install on the system.
   # Each repository will be listed only once, even if multiple packages from the same repository were used.
   # The same command (after variables substitution) is executed only once per transaction.
   post_transaction:*:in::/usr/bin/sh -c echo\ '${pkg.repo_id}'\ >>/tmp/actions-trans.log

   # ==============================================================================================
   # The next two actions emulate the DNF4 snapper plugin. It uses the "snapper" command-line proram.

   # Creates pre snapshot before the transaction and stores the snapshot number in the "tmp.snapper_pre_number" variable.
   pre_transaction::::/usr/bin/sh -c echo\ "tmp.snapper_pre_number=$(snapper\ create\ -t\ pre\ -p)"

   # If the variable "tmp.snapper_pre_number" exists, it creates post snapshot after the transaction and removes the variable "tmp.snapper_pre_number".
   post_transaction::::/usr/bin/sh -c [\ -n\ "${tmp.snapper_pre_number}"\ ]\ &&\ snapper\ create\ -t\ post\ --pre-number\ "${tmp.snapper_pre_number}"\ ;\ echo\ tmp.snapper_pre_number
