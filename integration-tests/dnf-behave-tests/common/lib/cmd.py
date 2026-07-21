# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

from behave.formatter.ansi_escapes import escapes
import subprocess

import consts


def run(cmd, shell=True, cwd=None):
    """
    Run a command.
    Return exitcode, stdout, stderr
    """

    proc = subprocess.Popen(
        cmd,
        shell=shell,
        cwd=cwd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
        errors="surrogateescape",
    )

    stdout, stderr = proc.communicate()
    return proc.returncode, stdout, stderr


def run_in_context(context, cmd, can_fail=False, expected_exit_code=None, **run_args):
    if getattr(context, "faketime", None) is not None:
        cmd = 'NO_FAKE_STAT=1 ' + context.faketime + cmd

    if getattr(context, "fake_kernel_release", None) is not None:
        cmd = context.fake_kernel_release + cmd

    if getattr(context, "lc_all", None) is not None:
        cmd = context.lc_all + cmd

    context.cmd = cmd

    if hasattr(context.scenario, "working_dir") and 'cwd' not in run_args:
        run_args['cwd'] = context.scenario.working_dir

    context.cmd_exitcode, context.cmd_stdout, context.cmd_stderr = run(cmd, **run_args)

    if not can_fail and context.cmd_exitcode != 0:
        raise AssertionError('Running command "%s" failed: %s' % (cmd, context.cmd_exitcode))
    elif expected_exit_code is not None and expected_exit_code != context.cmd_exitcode:
        raise AssertionError(
            'Running command "%s" had unexpected exit code: %s' % (cmd, context.cmd_exitcode)
        )


def assert_exitcode(context, exitcode):
    cmd = context.cmd.replace(consts.INVALID_UTF8_CHAR, "\\udcfd")
    assert context.cmd_exitcode == int(exitcode), \
        "Command has returned exit code {0}: {1}".format(context.cmd_exitcode, cmd)


def print_last_command(context):
    cmd = context.cmd.replace(consts.INVALID_UTF8_CHAR, "\\udcfd")
    if getattr(context, "cmd", ""):
        print(escapes["failed"])
        print("Last Command: %s%s" % (escapes["failed_arg"], cmd))
    if getattr(context, "cmd_stdout", ""):
        print(escapes["outline_arg"])
        print("Last Command stdout:%s" % (escapes['executing'], ))
        print(context.cmd_stdout.strip())
    if getattr(context, "cmd_stderr", ""):
        print(escapes["outline_arg"])
        print("Last Command stderr:%s" % (escapes['executing'], ))
        print(context.cmd_stderr.strip())

    print(escapes["reset"])
