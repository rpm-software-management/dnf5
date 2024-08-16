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

.. _dnf5_conf_todo-label:

Options that are documented/implemented in DNF but not in DNF5
==============================================================

`DNF5` has some options that are not yet implemented or documented.
This section marks the config options that are missing.
Some of the options might be duplicated in :ref:`DNF5 Configuration Reference<dnf5_conf-label>`.
In the event there are some duplicates, this section should
be considered invalid and the option implemented and documented.
If you find any issue, please, open a ticket at https://github.com/rpm-software-management/dnf5/issues.

This section does not track any deprecated option. For such options see :ref:`Deprecated Config Options<dnf5_conf_deprecated-label>`

[main] Options
==============

.. _arch_options-label:

``arch``

.. _autocheck_running_kernel_options-label:

``autocheck_running_kernel``
    :ref:`boolean <boolean-label>`

    Automatic check whether there is installed newer kernel module with security update than currently running kernel.

    Default: ``True``.

    .. NOTE::
       YUM compatibility option

.. _basearch_options-label:

``basearch``

.. _logfilelevel_options-label:

``logfilelevel``

.. _log_compress_options-label:

``log_compress``

.. _transaction_history_dir_options-label:

``transaction_history_dir``

.. _transformdb_options-label:

``transformdb``

.. _recent_options-label:

``recent``

.. _reset_nice_options-label:

``reset_nice``

.. _system_cachedir_options-label:

``system_cachedir``

.. _debuglevel_options-label:

``debuglevel``
    :ref:`integer <integer-label>`

    Debug messages output level, in the range ``0`` to ``10``. The higher the number the
    more debug output is put to stdout.

    Default: ``2``.

.. _debugdir_options-label:

``debugdir``

.. _diskspacecheck_options-label:

``diskspacecheck``
    :ref:`boolean <boolean-label>`

    If enabled, controls whether rpm should check available disk space during the transaction.

    Default: ``True``.

.. _errorlevel_options-label:

``errorlevel``
    :ref:`integer <integer-label>`

    Error messages output level, in the range ``0`` to ``10``. The higher the number the
    more error output is put to stderr.

    Default: ``3``.

    .. WARNING::
       This is deprecated in DNF and overwritten by -verbose commandline option.

.. _exit_on_lock_options-label:

``exit_on_lock``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will exit immediately when something else has the lock.

    Default: ``False``.

.. _gpgkey_dns_verification_options-label:

``gpgkey_dns_verification``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 will attempt to automatically verify GPG verification keys using the DNS
    system.

    This option requires the unbound python module (python3-unbound) to
    be installed on the client system. This system has two main features. The first
    one is to check if any of the already installed keys have been revoked. Automatic
    removal of the key is not yet available, so it is up to the user, to remove
    revoked keys from the system. The second feature is automatic verification
    of new keys when a repository is added to the system. In interactive mode, the
    result is written to the output as a suggestion to the user. In
    non-interactive mode (i.e. when -y is used), this system will automatically
    accept keys that are available in the DNS and are correctly signed using
    DNSSEC. It will also accept keys that do not exist in the DNS system and
    their NON-existence is cryptographically proven using DNSSEC. This is mainly to
    preserve backward compatibility.

    Default: ``False``.

.. _use_host_config_options-label:

``use_host_config``

.. _config_file_age_options-label:

``config_file_age``

.. _disable_excludes_options-label:

``disable_excludes``

.. _allow_downgrade_options-label:

``allow_downgrade``

.. _bugtracker_url_options-label:

``bugtracker_url``

.. _history_record_options-label:

``history_record``

.. _history_record_packages_options-label:

``history_record_packages``

.. _skip_broken_options-label:

``skip_broken``

.. _skip_unavailable_options-label:

``skip_unavailable``

.. _history_list_view_options-label:

``history_list_view``

.. _comment_options-label:

``comment``

.. _downloadonly_options-label:

``downloadonly``

.. _build_cache_options-label:

``build_cache``

.. _exclude_from_weak_options-label:

``exclude_from_weak``

.. _exclude_from_weak_autodetect_options-label:

``exclude_from_weak_autodetect``

.. _releasever_options-label:

``releasever``

.. _module_obsoletes_options-label:

``module_obsoletes``
    :ref:`boolean <boolean-label>`

    If enabled, DNF5 tries to apply modular obsoletes when possible.

    Default: ``False``.

.. _module_stream_switch_options-label:

``module_stream_switch``
    :ref:`boolean <boolean-label>`

    If enabled, allows switching enabled streams of a module.

    Default: ``False``.

.. _rpmverbosity_options-label:

``rpmverbosity``
    :ref:`string <string-label>`

    RPM debug scriptlet output level. One of: ``critical``, ``emergency``,
    ``error``, ``warn``, ``info`` or ``debug``.

    Default: ``info``.

.. _upgrade_group_objects_upgrade_options-label:

``upgrade_group_objects_upgrade``
    :ref:`boolean <boolean-label>`

    If enabled, performs the automatic running of ``group upgrade`` when running the ``upgrade`` command.

    Default: ``True``.

[main] Options - Colors
=======================

.. _color_options-label:

``color``
    :ref:`string <string-label>`

    Controls if DNF5 uses colored output on the command line.
    Possible values: ``auto``, ``never``, ``always``.

    Default: ``auto``.

.. _color_list_installed_older_options-label:

``color_list_installed_older``
    :ref:`color <color-label>`

    Color of installed packages that are older than any version among available packages.
    The option is used during list operations.

    Default: ``yellow``.

.. _color_list_installed_newer_options-label:

``color_list_installed_newer``
    :ref:`color <color-label>`

    Color of installed packages that are newer than any version among available packages.
    The option is used during list operations.

    Default: ``bold,yellow``.

.. _color_list_installed_reinstall_options-label:

``color_list_installed_reinstall``
    :ref:`color <color-label>`

    Color of installed packages that are among available packages and can be reinstalled.
    The option is used during list operations.

    Default: ``dim,cyan``.

.. _color_list_installed_extra_options-label:

``color_list_installed_extra``
    :ref:`color <color-label>`

    Color of installed packages that do not have any version among available packages.
    The option is used during list operations.

    Default: ``bold,red``.

Repo Options
============

.. _baseurl_repo_options-label:

``baseurl``

.. _cost_repo_options-label:

``cost``

.. _gpgcheck_options-label:

``gpgcheck``

.. _gpgkey_repo_options-label:

``gpgkey``

.. _metalink_repo_options-label:

``metalink``

.. _metadata_timer_sync_options-label:

``metadata_timer_sync``

.. _mirrorlist_repo_options-label:

``mirrorlist``

.. _module_hotfixes_repo_options-label:

``module_hotfixes``

.. _name_repo_options-label:

``name``

.. _priority_repo_options-label:

``priority``

.. _type_repo_options-label:

``type``


Repo Variables
==============

Options for both [main] and Repo
================================

.. _sslverifystatus_options-label:

``sslverifystatus``
