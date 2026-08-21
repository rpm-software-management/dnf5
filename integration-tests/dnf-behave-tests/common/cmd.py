# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave
import glob
import os

from common.lib.cmd import assert_exitcode, run, run_in_context
from common.lib.file import prepend_installroot


@behave.step("I set working directory to \"{working_dir}\"")
def i_set_working_directory(context, working_dir):
    context.scenario.working_dir = working_dir.format(context=context)


@behave.step("I execute \"{command}\" in \"{directory}\"")
def when_I_execute_command_in_directory(context, command, directory):
    run_in_context(context, command.format(context=context), cwd=directory.format(context=context))


@behave.step("I successfully execute \"{command}\" in \"{directory}\"")
def when_I_successfully_execute_command_in_directory(context, command, directory):
    when_I_execute_command_in_directory(context, command, directory)
    assert_exitcode(context, 0)


@behave.step("I execute \"{command}\"")
def when_I_execute_command(context, command):
    run_in_context(context, command.format(context=context))


@behave.step("I successfully execute \"{command}\"")
def when_I_successfully_execute_command(context, command):
    when_I_execute_command(context, command)
    assert_exitcode(context, 0)


@behave.step("I set LC_ALL to \"{value}\"")
def i_set_lc_all(context, value):
    context.lc_all = "LC_ALL={value} ".format(value=value)


@behave.step("I set umask to \"{octal_mode_str}\"")
def set_umask(context, octal_mode_str):
    os.umask(int(octal_mode_str, 8))


@behave.step("file \"{filepath}\" has mode \"{octal_mode_str}\"")
def file_has_mode(context, filepath, octal_mode_str):
    octal_mode = int(octal_mode_str, 8)
    matched_files = glob.glob(prepend_installroot(context, filepath))
    if len(matched_files) < 1:
        raise AssertionError("No files matching: {0}".format(filepath))
    if len(matched_files) > 1:
        raise AssertionError("Multiple files matching: {0} found:\n{1}" .format(filepath, '\n'.join(matched_files)))
    octal_file_mode = os.stat(matched_files[0]).st_mode & 0o777
    assert oct(octal_mode) == oct(octal_file_mode), \
        "File \"{}\" has mode \"{}\"".format(matched_files[0], oct(octal_file_mode))


@behave.step("file \"{filepath}\" has ACL entry \"{entry}\"")
def file_has_acl_entry(context, filepath, entry):
    filepath = prepend_installroot(context, filepath)
    command = ["/usr/bin/getfacl", "-c", filepath]
    ret, out, err = run(command, shell=False)
    if ret != 0:
        raise AssertionError("Could not retrieve ACL: Command \"{}\" failed: "
                             "{}".format(command.join(" "), err))
    for line in out.split("\n"):
        if line == entry:
            return
    raise AssertionError("File \"{}\" has ACL:\n{}".format(filepath, out))
