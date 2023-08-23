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
 Automatic command
##################

Synopsis
==========

``dnf5 automatic [options] [<config file>]``


Options
=======

``--timer``
    Apply random delay before execution.

``--downloadupdates``
    Automatically download updated packages.

``--no-downloadupdates``
    Do not automatically download updated packages.

``--installupdates``
    Automatically install downloaded updates (implies --downloadupdates).

``--no-installupdates``
    Do not automatically install downloaded updates.


Arguments
=========

``<config file>``
    Path to configuration file. Defaults to ``/etc/dnf/automatic.conf``.


Description
=============

Alternative CLI to ``dnf upgrade`` with specific facilities to make it suitable to be executed automatically and regularly from systemd timers, cron jobs and similar.

The operation of the tool is usually controlled by the configuration file or the function-specific timer units (see below). The command only accepts a single optional argument pointing to the config file, and some control arguments intended for use by the services that back the timer units. If no configuration file is passed from the command line, ``/etc/dnf/automatic.conf`` is used.

The tool synchronizes package metadata as needed and then checks for updates available for the given system and then either exits, downloads the packages or downloads and applies the updates. The outcome of the operation is then reported by a selected mechanism, for instance via the standard output, email or MOTD messages.

The systemd timer unit ``dnf-automatic.timer`` will behave as the configuration file specifies (see below) with regard to whether to download and apply updates. Some other timer units are provided which override the configuration file with some standard behaviours:

- dnf-automatic-notifyonly
- dnf-automatic-download
- dnf-automatic-install

Regardless of the configuration file settings, the first will only notify of available updates. The second will download, but not install them. The third will download and install them.


Run dnf-automatic
===================

You can select one that most closely fits your needs, customize ``/etc/dnf/automatic.conf`` for any specific behaviors, and enable the timer unit.

For example: ``systemctl enable --now dnf-automatic-notifyonly.timer``


Configuration File Format
===========================

The configuration file is separated into topical sections.

----------------------
``[commands]`` section
----------------------

Setting the mode of operation of the program.

``apply_updates``
    boolean, default: False

    Whether packages comprising the available updates should be applied by ``dnf-automatic.timer``, i.e. installed via RPM. Implies ``download_updates``. Note that if this is set to ``False``, downloaded packages will be left in the cache till the next successful DNF transaction. Note that the other timer units override this setting.

``download_updates``
    boolean, default: False

    Whether packages comprising the available updates should be downloaded by ``dnf-automatic.timer``. Note that the other timer units override this setting.

``network_online_timeout``
    time in seconds, default: 60

    Maximal time dnf-automatic will wait until the system is online. 0 means that network availability detection will be skipped.

``random_sleep``
    time in seconds, default: 0

    Maximal random delay before downloading.  Note that, by default, the ``systemd`` timers also apply a random delay of up to 1 hour.

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

``email_username``
    string, default empty.

    Username to use for SMTP server authentication.

``email_password``
    string, default empty.

    Password to use for SMTP server authentication.

------------------
``[base]`` section
------------------

Can be used to override settings from DNF's main configuration file. See :manpage:`dnf5-conf(5)`.
