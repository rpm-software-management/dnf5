..
    Copyright Contributors to the DNF5 project.
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

This plugin allows defining actions to be executed through libdnf5 callback hooks.
Each action is hooked to one specific callback. Actions for ``goal_resolved``, ``pre_transaction``, and
``post_transaction`` callbacks may define a (glob-like) filtering rule on the package
NEVRA or package files, as well as whether the package is incoming or outgoing.


Configuration
=============

The plugin does not extend the standard configuration. However, it reads "actions" files.

The actions files are read from the ``<libdnf5_plugins_config_dir>/actions.d/`` directory. Only files
with a ".actions" extension are read. Action files are read in lexical order (sorted by filename)
by comparing character values, ignoring locale.


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

  An empty filter means executing the command once with no information about the package.
  The "*" filter means executing the command for each package in the transaction that matches the ``direction`` filter.
  The filter can be non-empty only for ``goal_resolved``, ``pre_transaction``, and ``post_transaction`` callbacks.

``direction``
  Filters packages by their direction (coming into the system/going out of the system) in a transaction.
  If empty, the filter is not applied.
  The filter can only be non-empty when combined with a non-empty ``package_filter``.

  * ``in`` - packages coming to the system (downgrade, install, reinstall, upgrade)
  * ``out`` - packages going out of the system (upgraded, downgraded, reinstalled, removed, replaced/obsoleted)

``options``
  Options are separated by spaces. A space within an option can be written using escaping.

  * ``enabled=<value>`` - the ``<value>`` specifies when the action is enabled (added in version 0.3.0)

    * ``1`` - action is always enabled
    * ``host-only`` - the action is only enabled for operations on the host
    * ``installroot-only`` - the action is only enabled for operations in the alternative "installroot"

  * ``mode=<value>`` - the ``<value>`` specifies the action plugin communication mode (added in version 1.2.0)

    * ``plain`` - the original and default communication mode. Data is only passed to the process via arguments
                  when it is started. The process can send data to the actions plugin by writing to standard
                  output.
    * ``json`` - this mode establishes a bidirectional communication channel between the actions plugin and
                 the process, using a request-response model. The process writes requests to standard output
                 and reads responses from standard input. All communication is handled in JSON format (added
                 in version 1.2.0).

  * ``raise_error=<value>`` - the ``<value>`` specifies how action process errors are handled. What happens if
    the action process did not start, ended with a non-zero exit code, ended abnormally (received a signal),
    or an error occurred during communication (syntax error, communication interrupt, failed to process the output
    line in plain communication mode). If the option is not present, ``raise_error=0`` for backward compatibility
    (added in version 1.4.0).

    * ``0`` - the errors are logged
    * ``1`` - an exception is thrown

``command``
  Any executable file with arguments.

  Arguments are separated by spaces. A space within an argument can be written using escaping.
  Escaping can also be used to prevent substitution and to pass special characters: \\a, \\b, \\f, \\n, \\r, \\t, \\v.
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
    * ``${pkg.full_nevra}`` - package full nevra (epoch is always present) name-epoch:version-release.arch
    * ``${pkg.repo_id}`` - package repository id
    * ``${pkg.license}`` - package license
    * ``${pkg.location}`` - package relative path/location from repodata
    * ``${pkg.vendor}`` - package vendor
    * ``${pkg.action}`` - action performed on the package:

      * ``I`` - newly installed package
      * ``U`` - package installed as an upgrade
      * ``D`` - package installed as a downgrade
      * ``R`` - package used for reinstallation
      * ``E`` - erased (removed) package from the system
      * ``O`` - replaced package (was obsoleted/upgraded/downgraded/reinstalled/removed)
      * ``?`` - package with changed installation reason

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


Action process standard output format in "plain" communication mode
===================================================================

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

* ``conf.<option_name>=<value>`` - sets the value of option <option_name> in the base configuration

* ``conf.<repoid_pattern>.<option_name>=<value>`` - sets the value of option <option_name> in the matching repositories (added in version 1.1.0)

* ``var.<variable_name>=<value>`` - sets value of the variable <variable_name>

* ``stop=<message>`` - throws a stop exception with <message> (added in version 1.4.0)

* ``error=<message>`` - the error <message> is logged or throws an error exception with <message> if ``raise_error=1``
  (added in version 1.4.0)

* ``log.<level>=<message>`` - writes <message> to the logger with priority <level> (added in version 1.4.0)

    Levels: ``CRITICAL``, ``ERROR``, ``WARNING``, ``NOTICE``, ``INFO``, ``DEBUG``, ``TRACE``


Messages in "json" communication mode
=====================================

The "json" communication mode establishes a bidirectional communication channel between the actions plugin and the process, using a request-response model. The process writes requests to standard output and reads responses from the actions plugin from standard input. All communication is handled in JSON format.

*Format of a request message from the process:*

.. code-block:: json

  {"op":"<operation>", "domain":"<domain>", "args":{}}

*Format of a response:*

.. code-block:: json

  {"op":"reply", "requested_op":"<operation_from_request>", "domain":"<domain_from_request>",
    "status":"OK", "return":{}}

*Format of a response with an error message:*

.. code-block:: json

  {"op":"reply", "requested_op":"<operation_from_request>", "domain":"<domain_from_request>",
    "status":"ERROR", "message":"<error message>"}

Supported <operation> values
----------------------------

* ``get`` - request to get values
* ``set`` - request to set a value
* ``new`` - creates a new repository configuration
* ``log`` - writes a message to the logger (added in version 1.4.0)
* ``stop`` - throws a stop exception with a message (added in version 1.4.0)
* ``error`` - the error message is logged or throws an error exception with a message if ``raise_error=1`` (added in version 1.4.0)

Description of the ``get`` operation
------------------------------------
The ``get`` operation is defined for several ``<domain>``.

* ``conf`` - request to read the value of a configuration (global or repository configuration)
* ``vars`` - request to read the value of variables
* ``actions_vars`` - request to read the value of action plugin variables - these variables exist only in the actions plugin context
* ``actions_attrs`` - allows getting the process ID and the version of the actions plugin
* ``packages`` - request to list available and installed packages
* ``trans_packages`` - request to list packages in the transaction
* ``cmdline_packages_paths`` - request to list command-line packages

Reading a configuration value
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*Request format:*

.. code-block:: json

  {"op":"get", "domain":"conf", "args":{"key":"<name>"}}

  {"op":"get", "domain":"conf", "args":{"key":"<repo_id>.<name>"}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"get", "domain":"conf", "status":"OK",
    "return":{"keys_val":[{"key":"<name>", "value":"<value>"}]}}

  {"op":"reply", "requested_op":"get", "domain":"conf", "status":"OK",
    "return":{"keys_val":[{"key":"<repo_id>.<name>", "value":"<value>"}]}}

  {"op":"reply", "requested_op":"get", "domain":"conf", "status":"ERROR",
    "message":"<error message>"}

*Description:*

* ``<name>`` - the name of a global configuration option or a repository configuration option if ``<repo_id>`` is specified
* ``<repo_id>`` - the repository ID; in a request, it can contain globs, in which case the value of the configuration option is read for all matching repositories
* ``<value>`` - the read value of the configuration option
* ``<error message>`` - error message

When using ``<repo_id>``, the number of ``keys_val`` items in the response depends on the number of matching repositories.

*Example:*

.. code-block:: json

  {"op":"get", "domain":"conf", "args":{"key":"countme"}}
  {"op":"reply", "requested_op":"get", "domain":"conf", "status":"OK",
    "return":{"keys_val":[{"key":"countme", "value":"0"}]}}

  {"op": "get", "domain": "conf", "args": {"key": "*.enabled"}}
  {"op": "reply", "requested_op": "get", "domain": "conf", "status": "OK",
    "return": {
      "keys_val": [{"key": "dnf-ci-fedora.enabled", "value": "1"},
                   {"key": "dnf-ci-fedora-updates.enabled", "value": "1"},
                   {"key": "dnf-ci-thirdparty.enabled", "value": "0"},
                   {"key": "test-repo.enabled", "value": "0"}]}}

Reading a variable value
^^^^^^^^^^^^^^^^^^^^^^^^
*Request format:*

.. code-block:: json

  {"op":"get", "domain":"vars", "args":{"name":"<name>"}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"get", "domain":"vars", "status":"OK",
    "return":{"vars":[{"name":"<name>", "value":"<value>"}]}}

*Description:*

* ``<name>`` - the name of the variable; in a request, it can contain globs, in which case the value of all matching variables will be read
* ``<value>`` - the read value of the variable

The number of ``vars`` items in the response depends on the number of matching variables.

*Example:*

.. code-block:: json

  {"op":"get", "domain":"vars", "args":{"name":"test_var*"}}
  {"op":"reply", "requested_op":"get", "domain":"vars", "status":"OK",
    "return":{"vars":[{"name":"test_var1", "value":"value1"}]}}

  {"op":"get", "domain":"vars", "args":{"name":"nonexist_var"}}
  {"op":"reply", "requested_op":"get", "domain":"vars", "status":"OK",
    "return":{"vars":[]}}

Reading an action variable value
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*Request format:*

.. code-block:: json

  {"op":"get", "domain":"actions_vars", "args":{"name":"<name>"}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"get", "domain":"actions_vars", "status":"OK",
    "return":{"actions_vars":[{"name":"<name>", "value":"<value>"}]}}

*Description:*

* ``<name>`` - the name of the action variable; in a request, it can contain globs, in which case the value of all matching action variables will be read
* ``<value>`` - the read value of the action variable

The number of ``actions_vars`` items in the response depends on the number of matching action variables.

*Example:*

.. code-block:: json

  {"op":"get", "domain":"actions_vars", "args":{"name":"test_actions_var*"}}
  {"op":"reply", "requested_op":"get", "domain":"actions_vars", "status":"OK",
    "return":{"actions_vars":[{"name":"test_actions_var1", "value":"value1"}]}}

  {"op":"get", "domain":"actions_vars", "args":{"name":"nonexist_var"}}
  {"op":"reply", "requested_op":"get", "domain":"actions_vars", "status":"OK",
    "return":{"actions_vars":[]}}

Reading an action attribute value
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*Request format:*

.. code-block:: json

  {"op":"get", "domain":"actions_attrs", "args":{"key":"<name>"}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"get", "domain":"actions_attrs", "status":"OK",
    "return":{"actions_attrs":[{"key":"<name>", "value":"<value>"}]}}

*Description:*

* ``<name>`` - the name of the action attribute; in a request, it can contain globs, in which case the value of all matching actions plugin attributes will be read. Currently, two attributes are supported: ``pid`` and ``version``.

  * ``pid`` - the process ID of the actions plugin
  * ``version`` - the version of the actions plugin, a string in the format ``MAJOR.MINOR.MICRO``
* ``<value>`` - the read value of the action attribute

The number of ``actions_attrs`` items in the response depends on the number of matching action attributes.

*Example:*

.. code-block:: json

  {"op":"get", "domain":"actions_attrs", "args":{"key":"*"}}
  {"op":"reply", "requested_op":"get", "domain":"actions_attrs", "status":"OK",
    "return":{
      "actions_attrs":[{"key":"pid", "value":"523"},
                       {"key":"version", "value":"1.4.0"}]}}

  {"op":"get", "domain":"actions_attrs", "args":{"key":"nonexist_attribute"}}
  {"op":"reply", "requested_op":"get", "domain":"actions_attrs", "status":"OK",
    "return":{"actions_attrs":[]}}

Getting available, installed, and transaction packages
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``packages`` domain is used to get available and installed packages.
The ``trans_packages`` domain is used to get packages in the transaction.
The ``trans_packages`` domain can only be used in ``goal_resolved``, ``pre_transaction``, and ``post_transaction`` callbacks.

*Request format:*

.. code-block:: json

  {"op":"get", "domain":"packages", "args":{"output":["<pkg_attr>"]}}

  {"op":"get", "domain":"packages", "args":{"output":["<pkg_attr>"],
    "params":[{"key":"<param_name>"}]}}

  {"op":"get", "domain":"packages", "args":{"output":["<pkg_attr>"],
    "filters":[{"key":"<filter_name>", "value":"<value>", "operator":"<cmp_operator>"}]}}

  {"op":"get", "domain":"packages", "args":{"output":["<pkg_attr>"],
    "params":[{"key":"<param_name>"}],
    "filters":[{"key":"<filter_name>", "value":"<value>", "operator":"<cmp_operator>"}]}}

  {"op":"get", "domain":"trans_packages", "args":{"output":["<pkg_attr>"]}}

  {"op":"get", "domain":"trans_packages", "args":{"output":["<pkg_attr>"],
    "params":[{"key":"<param_name>"}]}}

  {"op":"get", "domain":"trans_packages", "args":{"output":["<pkg_attr>"],
    "filters":[{"key":"<filter_name>", "value":"<value>", "operator":"<cmp_operator>"}]}}

  {"op":"get", "domain":"trans_packages", "args":{"output":["<pkg_attr>"],
    "params":[{"key":"<param_name>"}],
    "filters":[{"key":"<filter_name>", "value":"<value>", "operator":"<cmp_operator>"}]}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"get", "domain":"packages", "status":"OK",
    "return":{"packages":[{"<pkg_attr>":"<attr_value>"}]}}

  {"op":"reply", "requested_op":"get", "domain":"trans_packages", "status":"OK",
    "return":{"trans_packages":[{"<pkg_attr>":"<attr_value>"}]}}

  {"op":"reply", "requested_op":"get", "domain":"packages", "status":"ERROR",
    "message":"<error message>"}

  {"op":"reply", "requested_op":"get", "domain":"trans_packages", "status":"ERROR",
    "message":"<error message>"}

*Description:*

* ``<pkg_attr>`` - package attribute to return; the array can contain multiple attributes

  General attributes

  * ``name`` - package name
  * ``arch`` - package arch
  * ``version`` - package version
  * ``release`` - package release
  * ``epoch`` - package epoch
  * ``na`` - package name.architecture
  * ``evr`` - package epoch-version-release
  * ``nevra`` - package nevra (0 epoch is hidden)
  * ``full_nevra`` - package full nevra (epoch is always present) name-epoch:version-release.arch
  * ``download_size`` - filesize of package
  * ``install_size`` - size the package should occupy after installing on disk
  * ``repo_id`` - package repository id
  * ``license`` - package license
  * ``location`` -  package relative path/location from repodata
  * ``vendor`` - package vendor

  Attributes usable only with the ``trans_packages`` domain

  * ``action`` - action performed on the package

    * ``I`` - newly installed package
    * ``U`` - package installed as an upgrade
    * ``D`` - package installed as a downgrade
    * ``R`` - package used for reinstallation
    * ``E`` - erased (removed) package from the system
    * ``O`` - replaced package (was obsoleted/upgraded/downgraded/reinstalled/removed)
    * ``?`` - package with changed installation reason

  * ``direction`` - package direction in transaction

   * ``IN`` - packages coming to the system (downgrade, install, reinstall, upgrade)
   * ``OUT`` - packages going out of the system (upgraded, downgraded, reinstalled, removed, replaced/obsoleted)

* ``<param_name>`` - one of ``IGNORE_EXCLUDES``, ``IGNORE_MODULAR_EXCLUDES``, ``IGNORE_REGULAR_EXCLUDES``, ``IGNORE_REGULAR_CONFIG_EXCLUDES``, ``IGNORE_REGULAR_USER_EXCLUDES``
* ``<filter_name>`` - name of package attribute to filter; the array can contain multiple attributes

  General attributes

  * ``name`` - filter packages by name, uses ``<value>`` and ``<cmp_operator>``
  * ``arch`` - filter packages by arch, uses ``<value>`` and ``<cmp_operator>``
  * ``version`` - filter packages by version, uses ``<value>`` and ``<cmp_operator>``
  * ``release`` - filter packages by release, uses ``<value>`` and ``<cmp_operator>``
  * ``epoch`` - filter packages by epoch, uses ``<value>`` and ``<cmp_operator>``
  * ``nevra`` - filter packages by nevra (0 epoch is hidden), uses ``<value>`` and ``<cmp_operator>``
  * ``repo_id`` - filter packages by repository id, uses ``<value>`` and ``<cmp_operator>``
  * ``available`` - filter available packages
  * ``installed`` - filter installed packages
  * ``userinstalled`` - filter user installed packages
  * ``installonly`` - filter installonly packages
  * ``description`` - filter packages by description, uses ``<value>`` and ``<cmp_operator>``
  * ``file`` - filter packages by files they contain, uses ``<value>`` and ``<cmp_operator>``
  * ``upgradable`` - filter installed packages for which there are available upgrades
  * ``upgrades`` - filter available packages that are upgrades to installed packages
  * ``downgradable`` - filter installed packages for which there are available downgrades
  * ``downgrades`` - filter available packages that are downgrades to installed packages

  Attributes usable only with the ``trans_packages`` domain

  * ``direction`` - filters packages by their direction in a transaction, uses ``<value>``; supported values:

   * ``IN`` - packages coming to the system (downgrade, install, reinstall, upgrade)
   * ``OUT`` - packages going out of the system (upgraded, downgraded, reinstalled, removed, replaced/obsoleted)

* ``<value>`` - a value used by some filters; the type of comparison depends on the ``<cmp_operator>``
* ``<cmp_operator>`` - the operator used by the filter to evaluate a match with ``<value>``; if not specified, ``EQ`` is assumed
* ``<attr_value>`` - the value of the package attribute
* ``<error message>`` - error message

The number of ``packages`` and ``trans_packages`` items in the response depends on the number of matching packages.

*Example:*

.. code-block:: json

  {"op":"get", "domain":"packages",
    "args":{
      "params":[{"key":"IGNORE_EXCLUDES"}],
      "filters":[{"key":"name", "value":"lame*", "operator":"GLOB"}],
      "output":["nevra"]}}
  {"op":"reply", "requested_op":"get", "domain":"packages", "status":"OK", "return":{"packages":[
    { "nevra":"lame-3.100-5.fc29.src" }, { "nevra":"lame-3.100-5.fc29.x86_64" },
    { "nevra":"lame-libs-3.100-5.fc29.x86_64" }, { "nevra":"lame-3.100-4.fc29.src" },
    { "nevra":"lame-3.100-4.fc29.x86_64" }, { "nevra":"lame-libs-3.100-4.fc29.x86_64"}]}}

  {"op":"get", "domain":"packages",
    "args":{
      "params":[{"key":"UNKNOWN"}],
      "filters":[{"key":"name", "value":"lame*", "operator":"GLOB"}],
      "output":["nevra"]}}
  {"op":"reply", "requested_op":"get", "domain":"packages", "status":"ERROR",
    "message":"Bad key \"UNKNOWN\" for params"}

  {"op":"get", "domain":"trans_packages",
    "args":{
      "filters":[{"key":"direction", "value":"IN"}, {"key":"arch", "value":"x86_64"}],
      "output":["action", "name", "version", "repo_id"]}}
  {"op":"reply", "requested_op":"get", "domain":"trans_packages", "status":"OK",
    "return":{
      "trans_packages":[
        {"action":"I", "name":"glibc", "version":"2.28", "repo_id":"dnf-ci-fedora-updates"},
        {"action":"I", "name":"glibc-all-langpacks", "version":"2.28", "repo_id":"dnf-ci-fedora-updates"},
        {"action":"I", "name":"glibc-common", "version":"2.28", "repo_id":"dnf-ci-fedora-updates"},
        {"action":"I", "name":"filesystem", "version":"3.9", "repo_id":"dnf-ci-fedora"}]}}

Getting paths of command-line specified packages
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*Request format:*

.. code-block:: json

  {"op":"get", "domain":"cmdline_packages_paths", "args":{}}

  {"op":"get", "domain":"cmdline_packages_paths", "args":{
    "filters":[{"key":"path", "value":"<value>"}]}}

  {"op":"get", "domain":"cmdline_packages_paths", "args":{
    "filters":[{"key":"path", "value":"<value>", "operator":"<cmp_operator>"}]}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"get", "domain":"cmdline_packages_paths", "status":"OK",
    "return":{"cmdline_packages_paths":["<pkg_path>"]}}

*Description:*

* ``<value>`` - if specified, it will only return paths matching the given pattern; the comparison type depends on ``<cmp_operator>``
* ``<cmp_operator>`` - the operator to evaluate whether the path matches the pattern in ``<value>``; if not specified, ``EQ`` is assumed
* ``<pkg_path>`` - the path to the package specified on the command line

The number of ``cmdline_packages_paths`` items in the response depends on the number of matching package paths on the command line.

*Example:*

.. code-block:: json

 {"op":"get", "domain":"cmdline_packages_paths",
   "args":{"filters":[{"key":"path", "value":"/local/*", "operator":"GLOB"}]}}
 {"op":"reply", "requested_op":"get", "domain":"cmdline_packages_paths", "status":"OK",
   "return":{"cmdline_packages_paths":["/local/packageB.rpm", "/local/packageC.rpm"]}}

Supported compare operators
^^^^^^^^^^^^^^^^^^^^^^^^^^^
Some ``get`` operations support ``<cmp_operator>``. The actions plugin supports the following compare operators:

* ``EQ`` - exact equality of arguments
* ``IEQ`` - string equality, case-insensitive comparison
* ``GT`` - greater than
* ``GTE`` - greater than or equal to
* ``LT`` - less than
* ``LTE`` - less than or equal to
* ``CONTAINS`` - contains a substring
* ``ICONTAINS`` - contains a substring, case-insensitive search
* ``STARTSWITH`` - starts with a substring
* ``ISTARTSWITH`` - starts with a substring, case-insensitive comparison
* ``ENDSWITH`` - ends with a substring
* ``IENDSWITH`` - ends with a substring, case-insensitive comparison
* ``REGEX`` - matches a regular expression
* ``IREGEX`` - matches a regular expression, case-insensitive evaluation
* ``GLOB`` - matches a glob pattern
* ``IGLOB`` - matches a glob pattern, case-insensitive evaluation

The meaning of any operator can be inverted by using the ``NOT_`` prefix. Examples: ``NOT_EQ``, ``NOT_GT``.


Description of the ``set`` operation
------------------------------------

The ``set`` operation is defined for several ``<domain>``.

* ``conf`` - request to set a configuration value (global or repository configuration)
* ``vars`` - request to set a variable value
* ``actions_vars`` - request to set an action plugin variable - these variables exist only in the actions plugin context


Setting a configuration value
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*Request format:*

.. code-block:: json

  {"op":"set", "domain":"conf", "args":{"key":"<name>", "value":"<value>"}}

  {"op":"set", "domain":"conf", "args":{"key":"<repo_id>.<name>", "value":"<value>"}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"set", "domain":"conf", "status":"OK",
    "return":{"keys_val":[{"key":"<name>", "value":"<value>"}]}}

  {"op":"reply", "requested_op":"set", "domain":"conf", "status":"OK",
    "return":{"keys_val":[{"key":"<repo_id>.<name>", "value":"<value>"}]}}

  {"op":"reply", "requested_op":"set", "domain":"conf", "status":"ERROR",
    "message":"<error message>"}

*Description:*

* ``<name>`` - the name of a global configuration option or a repository configuration option if ``<repo_id>`` is specified
* ``<repo_id>`` - the repository ID; in a request, it can contain globs, in which case the value of the configuration option is set for all matching repositories
* ``<value>`` - in the request, the value to be set; in the response, the actual value after being set
* ``<error message>`` - error message

When using ``<repo_id>``, the number of ``keys_val`` items in the response depends on the number of matching repositories.

*Example:*

.. code-block:: json

  {"op":"set", "domain":"conf", "args":{"key":"countme", "value":"1"}}
  {"op":"reply", "requested_op":"set", "domain":"conf", "status":"OK",
    "return":{"keys_val":[{"key":"countme", "value":"1"}]}}

  {"op":"set", "domain":"conf", "args":{"key":"dnf-ci-fedora*.enabled", "value":"1"}}
  {"op":"reply", "requested_op":"set", "domain":"conf", "status":"OK",
    "return":{
      "keys_val":[{"key":"dnf-ci-fedora.enabled", "value":"1"},
                  {"key":"dnf-ci-fedora-updates.enabled", "value":"1"}]}}

Setting a variable value
^^^^^^^^^^^^^^^^^^^^^^^^
*Request format:*

.. code-block:: json

  {"op":"set", "domain":"vars", "args":{"name":"<name>", "value":"<value>"}}

  {"op":"set", "domain":"vars", "args":{"name":"<name>"}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"set", "domain":"vars", "status":"OK",
    "return":{"vars":[{"name":"<name>", "value":"<value>"}]}}

  {"op":"reply", "requested_op":"set", "domain":"vars", "status":"OK",
    "return":{"vars":[{"name":"<name>"}]}}

  {"op":"reply", "requested_op":"set", "domain":"vars", "status":"ERROR",
    "message":"<error message>"}

*Description:*

* ``<name>`` - the name of the variable
* ``<value>`` - in the request, the value to be set; if not specified, it means the variable should be removed. In the response, the actual value after being set; if not specified, the variable did not exist or was removed
* ``<error message>`` - error message

*Example:*

.. code-block:: json

  {"op":"set", "domain":"vars", "args":{"name":"test_var1", "value":"value1"}}
  {"op":"reply", "requested_op":"set", "domain":"vars", "status":"OK",
    "return":{"vars":[{"name":"test_var1", "value":"value1"}]}}

Setting an action variable value
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*Request format:*

.. code-block:: json

  {"op":"set", "domain":"actions_vars", "args":{"name":"<name>", "value":"<value>"}}

  {"op":"set", "domain":"actions_vars", "args":{"name":"<name>"}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"set", "domain":"actions_vars", "status":"OK",
    "return":{"vars":[{"name":"<name>", "value":"<value>"}]}}

  {"op":"reply", "requested_op":"set", "domain":"actions_vars", "status":"OK",
    "return":{"vars":[{"name":"<name>"}]}}

  {"op":"reply", "requested_op":"set", "domain":"actions_vars", "status":"ERROR",
    "message":"<error message>"}

*Description:*

* ``<name>`` - the name of the action variable
* ``<value>`` - in the request, the value to be set; if not specified, it means the variable should be removed. In the response, the actual value after being set; if not specified, the variable did not exist or was removed
* ``<error message>`` - error message

*Example:*

.. code-block:: json

  {"op":"set", "domain":"actions_vars", "args":{"name":"test_actions_var1", "value":"value1"}}
  {"op":"reply", "requested_op":"set", "domain":"actions_vars", "status":"OK",
    "return":{"actions_vars":[{"name":"test_actions_var1", "value":"value1"}]}}

Description of the ``new`` operation
------------------------------------

The ``new`` operation creates a new repository configuration. It can be used only in the ``repos_configured`` callback.

*Request format:*

.. code-block:: json

  {"op":"new", "domain":"repoconf",
    "args":{"keys_val":[{"key":"repo_id", "value":"<repo_id>"},
                        {"key":"<repo_opt>", "value":"<opt_value>"}]}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"new", "domain":"repoconf", "status":"OK",
    "return":{"keys_val":[{"key":"repo_id", "value":"<repo_id>"},
                          {"key":"<repo_opt>", "value":"<opt_value>"}]}}

  {"op":"reply", "requested_op":"new", "domain":"repoconf", "status":"ERROR",
    "message":"<error_message>"}

*Description:*

* ``<repo_id>`` - repository ID
* ``<repo_opt>`` - any existing repository configuration option
* ``<opt_value>`` - the value to be set for the configuration option

The ``keys_val`` field must contain at least one element - specifying the ``repo_id`` is mandatory. Setting other repository configuration options is optional. If the ``enabled`` option is not present, the repository is disabled by default. The response contains the same keys as the request and their actual values.

*Example:*

.. code-block:: json

  {"op":"new", "domain":"repoconf",
    "args":{
      "keys_val":[{"key":"repo_id", "value":"test-repo"},
                  {"key":"name", "value":"Test repository"},
                  {"key":"enabled", "value":"false"},
                  {"key":"baseurl", "value":"https://xyz.com/rpm"}]}}
  {"op":"reply", "requested_op":"new", "domain":"repoconf", "status":"OK",
    "return":{
      "keys_val":[{"key":"repo_id", "value":"test-repo"},
                  {"key":"name", "value":"Test repository"},
                  {"key":"enabled", "value":"0"},
                  {"key":"baseurl", "value":"https://xyz.com/rpm"}]}}

Description of the ``log`` operation
------------------------------------

The ``log`` operation writes a message to the logger.

*Request format:*

.. code-block:: json

  {"op":"log", "args":{"level":"<level>", "message":"<message>"}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"log", "domain":"log", "status":"OK"}

  {"op":"reply", "requested_op":"log", "domain":"log", "status":"ERROR",
    "message":"Unknown log level '<level_from_request>'"}

*Description:*

* ``<level>`` - ``CRITICAL``, ``ERROR``, ``WARNING``, ``NOTICE``, ``INFO``, ``DEBUG``, ``TRACE``
* ``<message>`` - text to be written to the log
* ``<level_from_request>`` - the level from the request, if it contains an unsupported value

*Example:*

.. code-block:: json

  {"op":"log", "args":{"level":"WARNING", "message":"My warning message"}}
  {"op":"reply", "requested_op":"log", "domain":"log", "status":"OK"}

Description of the ``stop`` operation
-------------------------------------

The ``stop`` operation throws a stop exception with a message.

*Request format:*

.. code-block:: json

  {"op":"stop", "args":{"message":"<message>"}}

*Description:*

* ``<message>`` - text to be included in the stop exception

After this request, the application is expected to terminate. The actions plugin does not send a response but closes the communication channel - pipes connected to the standard output and input of the process.

*Example:*

.. code-block:: json

  {"op":"stop", "args":{"message":"I want to stop the task"}}

Description of the ``error`` operation
--------------------------------------

The ``error`` operation logs the error message or throws an error exception with a message if ``raise_error=1``.

*Request format:*

.. code-block:: json

  {"op":"error", "args":{"message":"<message>"}}

*Response format:*

.. code-block:: json

  {"op":"reply", "requested_op":"error", "domain":"error", "status":"OK"}

*Description:*

* ``<message>`` - text written to the log or included in the error exception if ``raise_error=1``

If ``raise_error=1``, the application is expected to terminate. The actions plugin does not send a response but closes the communication channel - pipes connected to the standard output and input of the process.

*Example:*

.. code-block:: json

  {"op":"error", "args":{"message":"Error in action process 1"}}
  {"op":"reply", "requested_op":"error", "domain":"error", "status":"OK"}


An example actions file:
========================
.. code-block::

  # Prints header with process id
  pre_base_setup::::/usr/bin/sh -c echo\ -------------------------------------\ >>/tmp/actions-trans.log
  pre_base_setup::::/usr/bin/sh -c date\ >>/tmp/actions-trans.log
  pre_base_setup::::/usr/bin/sh -c echo\ libdnf5\ pre_base_setup\ was\ called.\ Process\ ID\ =\ '${pid}'.\ >>/tmp/actions-trans.log
  pre_base_setup:::enabled=installroot-only:/usr/bin/sh -c echo\ run\ in\ alternative\ "installroot":\ installroot\ =\ '${conf.installroot}'\ >>/tmp/actions-trans.log

  # Prints the value of the configuration option "defaultyes".
  pre_base_setup::::/usr/bin/sh -c echo\ pre_base_setup:\ conf.defaultyes=${conf.defaultyes}\ >>/tmp/actions.log

  # Prints a message that the "post_base_setup" callback was called.
  post_base_setup::::/usr/bin/sh -c echo\ libdnf5\ post_base_setup\ was\ called.\ >>/tmp/actions-trans.log

  # Executes the "add_new_repo" application with json communication.
  # This application, for instance, can add new repository configurations.
  repos_configured:::mode=json:/usr/local/bin/add_new_repo

  # Prints a list of configured repositories with their enable state.
  repos_configured::::/usr/bin/sh -c echo\ Repositories:\ ${conf.*.enabled}\ >>/tmp/repos.log

  # Prints a list of repositories that use the http protocol in baseurl.
  repos_configured::::/usr/bin/sh -c echo\ "${conf.*.baseurl=*http://*}"\ >>/tmp/baseurl_http.log

  # Disables all repositories whose id starts with "rpmfusion".
  repos_configured::::/usr/bin/sh -c echo\ conf.rpmfusion*.enabled=0

  # Executes the "check_transaction" application with json communication, terminating if the application encounters an error.
  # This application, for instance, can check for forbidden packages within a transaction and send a stop message.
  pre_transaction:::mode=json raise_error=1:/usr/local/bin/check_transaction

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
  # The next two actions emulate the DNF4 snapper plugin. It uses the "snapper" command-line program.

  # Creates a snapshot description and saves it to the "tmp.snapper_descr" variable.
  pre_transaction::::/usr/bin/sh -c echo\ "tmp.snapper_descr=$(ps\ -o\ command\ --no-headers\ -p\ '${pid}')"

  # Creates pre snapshot before the transaction and stores the snapshot number in the "tmp.snapper_pre_number" variable.
  pre_transaction::::/usr/bin/sh -c echo\ "tmp.snapper_pre_number=$(snapper\ create\ -t\ pre\ -p\ -d\ '${tmp.snapper_descr}')"

  # If the variable "tmp.snapper_pre_number" exists, it creates post snapshot after the transaction and removes the used variables.
  post_transaction::::/usr/bin/sh -c [\ -n\ "${tmp.snapper_pre_number}"\ ]\ &&\ snapper\ create\ -t\ post\ --pre-number\ "${tmp.snapper_pre_number}"\ -d\ "${tmp.snapper_descr}"\ ;\ echo\ tmp.snapper_pre_number\ ;\ echo\ tmp.snapper_descr
