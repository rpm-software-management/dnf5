# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave
import sys
import pexpect

from lib.rpmdb import get_rpmdb_rpms


def stdout_from_shell(context):
    # Few notes:
    #   1. replacing CR/LF with just LF
    #   2. at the begining, there might also be TTY echo (the command that was sent)
    context.cmd_stdout = context.shell_session.before.decode().replace("\r\n", "\n")


def expect_from_shell(context, shell_out):
    try:
        context.shell_session.expect(shell_out)
    except pexpect.exceptions.EOF:
        stdout_from_shell(context)
        raise AssertionError("expected: '" + str(shell_out) + "' was not found in dnf shell output,"
                             " it terminated unexpectedly")
    except pexpect.exceptions.TIMEOUT:
        stdout_from_shell(context)
        raise AssertionError("expected: '" + str(shell_out) + "' was not found in dnf shell output,"
                             " it timed out")


@behave.step("I open dnf shell session")
def when_I_open_dnf_shell(context):
    cmd = " ".join(context.dnf.get_cmd(context)) + " shell"
    context.cmd = cmd
    context.dnf["rpmdb_pre"] = get_rpmdb_rpms(context.dnf.installroot)

    context.shell_session = pexpect.spawn(cmd, env={"COLORTERM": "FALSE"})
    # pexpect adds a short delay before sending data, so that SSH has time
    # to turn off TTY echo; the echo is still there though, so removing the delay
    context.shell_session.delaybeforesend = None

    expect_from_shell(context, '> ')
    stdout_from_shell(context)


@behave.step("I execute in dnf shell \"{command}\"")
def when_I_execute_in_shell(context, command):
    if context.shell_session is None:
        raise AssertionError("dnf shell session must be opened first")

    context.dnf["rpmdb_pre"] = get_rpmdb_rpms(context.dnf.installroot)

    context.shell_session.sendline(command.format(context=context))

    if command.strip() == "quit" or command.strip() == "exit":
        expect_from_shell(context, pexpect.EOF)
        stdout_from_shell(context)
        context.shell_session = None
        return

    expect_from_shell(context, "\r\n[^ \r-]*> ")
    stdout_from_shell(context)
