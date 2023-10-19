..
    Copyright Contributors to the libdnf project.

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

.. _aliases_misc_ref-label:

##################################
Aliases for command line arguments
##################################

Description
===========

It is possible to define custom aliases which can be then used as dnf commands or options to abbreviate longer
command and option sequences.

The aliases can be configured in the toml format and the configuration files are taken from these drop-in
directories:

  - ``/usr/share/dnf5/aliases.d/``
  - ``/etc/dnf/dnf5-aliases.d/``
  - ``$XDG_CONFIG_HOME/dnf5/aliases.d/``


Syntax:
-------

Configuration file must begin with the attribute ``version`` with a value of a supported version, for example:

.. code-block:: none

    version = '1.0'

Each alias is defined in a separate section, using ``key = value`` pairs, for example:

.. code-block:: none

    ['in']
    type = 'command'
    attached_command = 'install'
    descr = "Alias for 'install'"

The section name uniquely identifies the alias. It is in a form of a path, i.e. one or more strings separated by dots,
which defines the scope of the alias. For example, string "group.list.in" would identify an alias usable only within
the scope of the group list subcommand.

There are the following types of aliases:

    - :ref:`command <aliases_misc_command_ref-label>`
    - :ref:`cloned_named_arg <aliases_misc_cloned_named_arg_ref-label>`
    - :ref:`named_arg <aliases_misc_named_arg_ref-label>`
    - :ref:`group <aliases_misc_group_ref-label>`


.. _aliases_misc_command_ref-label:

Type: command
-------------

The ``command`` alias defines an alias for a command.

Keys:
    - ``type`` - Must have value ``command``.
    - ``attached_command`` - Path to a command for which this alias is defined.
    - ``descr`` - Description that will be shown in help.
    - ``group_id`` - A group this alias is part of if any.
    - ``complete`` - Whether bash autocompletion should be used for this alias, default is false.
    - ``attached_named_args`` - Options that will be used with the command. The format is an array of inline
      tables, each of which must contain an ``id_path`` key to specify the path to an option, and may contain also
      a ``value`` key to specify the value of the option.

The required keys are ``type``, and ``attached_command``.

Examples:
  - Alias ``grouplist`` for ``group list``:

    .. code-block:: none

        ['grouplist']
        type = 'command'
        attached_command = 'group.list'
        descr = "Alias for 'group list'"

  - Alias ``group.ls`` for ``group list``:

    .. code-block:: none

        ['group.ls']
        type = 'command'
        attached_command = 'group.list'
        descr = "Alias for 'group list'"
        complete = true

  - Alias ``list-fedora-all`` for ``--repo=fedora list --showduplicates``:

    .. code-block:: none

        ['list-fedora-all']
        type = 'command'
        attached_command = 'list'
        descr = "Alias for '--repo=fedora list --showduplicates'"
        complete = true
        attached_named_args = [
            { id_path = 'repo', value = 'fedora' },
            { id_path = 'list.showduplicates' }
        ]


.. _aliases_misc_cloned_named_arg_ref-label:

Type: cloned_named_arg
----------------------

The ``cloned_named_arg`` alias defines another name for a given option.

Keys:
    - ``type`` - Must have value ``cloned_named_arg``.
    - ``long_name`` - Name of the alias option.
    - ``short_name`` - One-letter shortcut of the name.
    - ``source`` - Path to the option for which this alias is defined.
    - ``group_id`` - A group this alias is part of if any.
    - ``complete`` - Whether bash autocompletion should be used for this alias, default is false.

The required keys are ``type``, either ``long_name`` or ``short_name``, and ``source``.

Examples:
  - Alias ``--nobest`` for ``--no-best``:

    .. code-block:: none

        ['nobest']
        type = 'cloned_named_arg'
        long_name = 'nobest'
        source = 'no-best'

  - Alias ``repoquery --list`` or ``repoquery -l`` for ``repoquery --files``:

    .. code-block:: none

        ['repoquery.list']
        type = 'cloned_named_arg'
        long_name = 'list'
        short_name = 'l'
        source = 'repoquery.files'


.. _aliases_misc_named_arg_ref-label:

Type: named_arg
---------------

The ``named_arg`` defines an alias that can replace multiple options and can define a value for each.

Keys:
    - ``type`` - Must have value ``named_arg``.
    - ``long_name`` - Name of the alias option.
    - ``short_name`` - One-letter shortcut of the name.
    - ``descr`` - Description that will be shown in help.
    - ``has_value`` - Whether the option requires a value. The value is then substituted for ``${}`` strings in the
      values of ``attached_named_args``. Default is false.
    - ``value_help`` - The string shown in help for the value (e.g. ``CONFIG_FILE_PATH`` for
      ``--config=CONFIG_FILE_PATH``).
    - ``const_value`` - Default constant value (specified only if the alias does not have a value on the command line).
      The value is then substituted for ``${}`` strings in the values of ``attached_named_args``.
    - ``group_id`` - A group this alias is part of if any.
    - ``complete`` - Whether bash autocompletion should be used for this alias, default is false.
    - ``attached_named_args`` - Options that will be used. The format is an array of inline tables, each of which must
      contain an ``id_path`` key to specify the path to an option, and may contain also a ``value`` key to specify the
      value of the option.

The required keys are ``type``, and either ``long_name`` or ``short_name``.

Examples:
  - Alias ``list --all-available`` for ``list --showduplicates --available``:

    .. code-block:: none

        ['list.all-available']
        type = 'named_arg'
        long_name = 'all-available'
        attached_named_args = [
            { id_path = 'list.showduplicates' },
            { id_path = 'list.available' }
        ]

  - Alias ``download --dest=DESTDIR`` for ``download --destdir=DESTDIR``:

    .. code-block:: none

        ['download.dest']
        type = 'named_arg'
        long_name = 'dest'
        has_value = true
        value_help = 'DESTDIR'
        attached_named_args = [
            { id_path = 'download.destdir' }
        ]

  - Alias ``--settsflags=TS_FLAGS`` for ``--setopt=tsflags=TS_FLAGS``:

    .. code-block:: none

        ['settsflags']
        type = 'named_arg'
        long_name = 'settsflags'
        descr = 'Set transaction flags'
        has_value = true
        value_help = 'TS_FLAGS'
        attached_named_args = [
         { id_path = 'setopt', value = 'tsflags=${}' }
        ]


.. _aliases_misc_group_ref-label:

Type: group
-----------

The ``group`` defines a group for multiple commands or options.

Required keys:
    - ``type`` - Must have value ``group``.
    - ``header`` - The header of the group as will be shown in help.

The required keys are ``type`` and ``header``.

The aliases are added to the group using the ``group_id`` key in their respective sections.

Examples:
  - Group ``query-aliases`` for subcommand ``repo`` containing aliases ``repo.ls`` and ``repo.if``:

    .. code-block:: none

        ['repo.query-aliases']
        type = 'group'
        header = 'Query Aliases:'

        ['repo.ls']
        type = 'command'
        attached_command = 'repo.list'
        descr = "Alias for 'repo list'"
        group_id = 'query-aliases'

        ['repo.if']
        type = 'command'
        attached_command = 'repo.info'
        descr = "Alias for 'repo info'"
        group_id = 'query-aliases'
