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

.. _automatic_plugin_ref-label:

##################
 Automatic Command
##################

Synopsis
========

``dnf5 automatic [options]``


Description
===========

Alternative CLI to ``dnf upgrade`` with specific facilities to make it suitable to be executed automatically and regularly from systemd timers, cron jobs and similar.

The operation of the tool is controlled by configuration files. Default values are set from ``/usr/share/dnf5/dnf5-plugins/automatic.conf`` config file. Host-specific overrides from ``/etc/dnf/automatic.conf`` are then applied.

The tool synchronizes package metadata as needed and then checks for updates available for the given system and then either exits, downloads the packages or downloads and applies the updates. The outcome of the operation is then reported by a selected mechanism, for instance via the standard output, email or MOTD messages.

The systemd timer unit ``dnf5-automatic.timer`` will behave as the configuration file specifies (see below) with regard to whether to download and apply updates.


Options
=======

``--timer``
    | Apply random delay before execution.

The following options can be used to override values from the configuration file.

``--downloadupdates``
    | Automatically download updated packages.

``--no-downloadupdates``
    | Do not automatically download updated packages.

``--installupdates``
    | Automatically install downloaded updates (implies --downloadupdates).

``--no-installupdates``
    | Do not automatically install downloaded updates.



Run dnf5 automatic service
==========================

The service is typically executed using the systemd timer ``dnf5-automatic.timer``. To configure the service, customize the ``/etc/dnf/automatic.conf`` file. You can either copy the distribution config file from ``/usr/share/dnf5/dnf5-plugins/automatic.conf`` and use it as a baseline, or create your own configuration file from scratch with only the required overrides.

Then enable the timer unit:

``systemctl enable --now dnf5-automatic.timer``


Configuration File Format
=========================

The configuration file is separated into topical sections.


----------------------
``[commands]`` section
----------------------

Setting the mode of operation of the program.

``apply_updates``
    boolean, default: False

    Whether packages comprising the available updates should be applied by ``dnf5-automatic.timer``, i.e. installed via RPM. Implies ``download_updates``. Note that if this is set to ``False``, downloaded packages will be left in the cache till the next successful DNF transaction.

``download_updates``
    boolean, default: True

    Whether packages comprising the available updates should be downloaded by ``dnf5-automatic.timer``.

``network_online_timeout``
    time in seconds, default: 60

    Maximal time ``dnf5 automatic`` will wait until the system is online. 0 means that network availability detection will be skipped.

``random_sleep``
    time in seconds, default: 0

    Maximal random delay before downloading (only applied if ``--timer`` option was used).  Note that, by default, the ``systemd`` timers also apply a random delay of up to 1 hour.

.. _upgrade_type_automatic-label:

``upgrade_type``
    either one of ``default``, ``security``, default: ``default``

    What kind of upgrades to look at. ``default`` signals looking for all available updates, ``security`` only those with an issued security advisory.

``reboot``
    either one of ``never``, ``when-changed``, ``when-needed``, default: ``never``

    When the system should reboot following upgrades. ``never`` does not reboot the system. ``when-changed`` triggers a reboot after any upgrade. ``when-needed`` triggers a reboot only when rebooting is necessary to apply changes, such as when systemd or the kernel is upgraded.

``reboot_command``
    string, default: ``shutdown -r +5 'Rebooting after applying package updates'``

    Specify the command to run to trigger a reboot of the system. For example, to skip the 5-minute delay and wall message, use ``shutdown -r``


----------------------
``[emitters]`` section
----------------------

Choosing how the results should be reported.

.. _emit_via_automatic-label:

``emit_via``
    list, default: ``stdio``

    List of emitters to report the results through. Available emitters are ``stdio`` to print the result to standard output, ``command`` to send the result to a custom command, ``command_email`` to send an email using a command, ``email`` to send the report via email using SMTP sever, and ``motd`` sends the result to */etc/motd.d/dnf5-automatic* file.

``system_name``
    string, default: hostname of the given system

    How the system is called in the reports.

``emit_no_updates``
    boolean, default: False

    Whether to emit a message when nothing interesting happened - the operation succeeded and no packages were available/installed.


---------------------
``[command]`` section
---------------------

The command emitter configuration. Variables usable in format string arguments are ``body`` with the message body.

``command_format``
    format string, default: ``cat``

    The shell command to execute.

``stdin_format``
    format string, default: ``{body}``

    The data to pass to the command on stdin.


---------------------------
``[command_email]`` section
---------------------------

The command email emitter configuration. Variables usable in format string arguments are ``body`` with message body, ``subject`` with email subject, ``email_from`` with the "From:" address and ``email_to`` with a space-separated list of recipients.

``command_format``
    format string, default: ``mail -Ssendwait -s {subject} -r {email_from} {email_to}``

    The shell command to execute.

``email_from``
    string, default: ``root``

    Message's "From:" address.

``email_to``
    list, default: ``root``

    List of recipients of the message.

``stdin_format``
    format string, default: ``{body}``

    The data to pass to the command on stdin.


-------------------
``[email]`` section
-------------------

The email emitter configuration.

``email_from``
    string, default: ``root``

    Message's "From:" address.

``email_to``
    list, default: ``root``

    List of recipients of the message.

``email_host``
    string, default: ``localhost``

    Hostname of the SMTP server used to send the message.

``email_port``
    integer, default: ``25``

    Port number to connect to at the SMTP server.

``email_tls``
    either one of ``no``, ``yes``, ``starttls``, default: ``no``

    Whether to use TLS, STARTTLS or no encryption to connect to the SMTP server.


------------------
``[base]`` section
------------------

Can be used to override settings from DNF's main configuration file. See :manpage:`dnf5-conf(5)`.
