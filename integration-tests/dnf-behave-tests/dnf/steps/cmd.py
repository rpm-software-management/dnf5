# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave
import os
import re
import subprocess
import sys
import time
from datetime import datetime
import json

from common.lib.cmd import assert_exitcode, run_in_context
from common.lib.file import prepend_installroot
from fixtures import start_server_based_on_type
from lib.rpmdb import get_rpmdb_rpms
from lib.json import diff_json_pattern_values


def get_boot_time():
    """Return the boot time of this system (as a timestamp)."""
    return int(os.stat('/proc/1/cmdline').st_mtime)


def extract_section_content_from_text(section_header, text):
    SECTION_HEADERS = [
        'Installing:', 'Upgrading:', 'Removing:', 'Downgrading:', 'Installing dependencies:',
        'Removing unused dependencies:',  # dnf install/remove... transaction listing
        'Installed:', 'Upgraded:', 'Removed:', 'Downgraded:',  # dnf install/remove/... result
        'Installed Packages', 'Available Packages', 'Recently Added Packages'  # dnf list
    ]
    parsed = ''
    copy = False
    for line in text.split('\n'):
        if (not copy) and section_header == line:
            copy = True
            continue
        if copy:  # copy lines until hitting empty line or another known header
            if line.strip() and line not in SECTION_HEADERS:
                parsed += "%s\n" % line
            else:
                return parsed
    return parsed


@behave.step("I execute step \"{step}\"")
def execute_step(context, step):
    context.execute_steps(step)


@behave.step("I set environment variable \"{var}\" to \"{value}\"")
def set_environment_variable(context, var, value):
    os.environ[var] = value.format(context=context)


@behave.step("I move the clock {direction} to \"{when}\"")
def faketime(context, direction, when):
    assert os.path.exists('/usr/bin/faketime'), 'Faketime binary must be installed'
    if when == 'before boot-up':
        stamp = get_boot_time() - 24 * 60 * 60  # 1 day before boot-up
        time = datetime.utcfromtimestamp(stamp)
        assert direction == 'backward', 'Boot time is always in the past'
    elif when == 'the present':
        context.faketime = None
        return
    else:
        time = when
    context.faketime = "faketime '%s' " % time


@behave.step("today is {when}")
def faketime_today(context, when):
    context.execute_steps('when I move the clock backward to "{}"'.format(when))


@behave.step("I fake kernel release to {release}")
def i_fake_kernel_release(context, release):
    """Override uname() system call to return {release} as release.

    This is useful for faking running kernel, because libdnf determines running kernel in the
    following way:
        1. makes uname syscall to get operation system release
        2. searches for file "/boot/vmlinuz-<release>" (<release> from previous step)
        3. searches for RPM that provides that file
           -> this RPM is considered the running kernel

    Note that libdnf only checks running kernel when not running in installroot, therefore,
    @no_installroot tag is needed.

    By faking uname.release, all that remains to create fake running-kernel package is for the
    RPM to provide file "/boot/vmlinuz-{release}". (Do not forget to add the file into the %files
    section in the .spec file. It can be specified as %ghost for easier build.)

    :param release: arbitrary string
    """
    assert os.path.exists('/usr/bin/fakeuname'), (
        'Fakeuname binary must be installed (provided by fakeuname from '
        'rpmsoftwaremanagement/test-utils copr repo)')
    context.fake_kernel_release = "fakeuname {} ".format(release)


@behave.step("I stop faking kernel release")
def i_stop_faking_kernel_release(context):
    """Cancels the effect of "I fake kernel release to {release}" step."""
    context.fake_kernel_release = None


@behave.step("I execute dnf with args \"{args}\"")
def when_I_execute_dnf_with_args(context, args):
    cmd = " ".join(context.dnf.get_cmd(context))
    cmd += " " + args.format(context=context)
    context.dnf["rpmdb_pre"] = get_rpmdb_rpms(context.dnf.installroot)
    run_in_context(context, cmd, can_fail=True)


@behave.step("I execute dnf with args \"{args}\" as an unprivileged user")
def when_I_execute_dnf_with_args_unprivileged(context, args):
    """
    Creates an unpriviged user if it doesn't exist (therefore the test is
    destructive!). Runs the dnf command under this user.
    """
    # create a regular user for the test if it doesn't exist
    run_in_context(context, 'id -u testuser || useradd testuser', can_fail=False)

    cmd = " ".join(context.dnf.get_cmd(context))
    cmd += " " + args.format(context=context)
    context.dnf["rpmdb_pre"] = get_rpmdb_rpms(context.dnf.installroot)

    # escape the quotes so that we can wrap the command in `su`
    cmd = cmd.replace('"', '\\"')
    run_in_context(context, 'su testuser -c "' + cmd + '"', can_fail=True)


@behave.step("I execute dnf with args \"{args}\" {times} times")
def when_I_execute_dnf_with_args_times(context, args, times):
    for i in range(int(times)):
        context.execute_steps('when I execute dnf with args "{}"'.format(args))


@behave.step("I execute rpm with args \"{args}\"")
def when_I_execute_rpm_with_args(context, args):
    cmd = "rpm --root=" + context.dnf.installroot
    cmd += " " + args.format(context=context)
    run_in_context(context, cmd, can_fail=True)


@behave.step("I execute rpm on host with args \"{args}\"")
def when_I_execute_rpm_on_host_with_args(context, args):
    cmd = "rpm"
    cmd += " " + args.format(context=context)
    run_in_context(context, cmd, can_fail=True)


@behave.step("I execute dnf-automatic with args \"{args}\"")
def when_I_execute_dnf_automatic_with_args(context, args):
    cmd = "dnf-automatic"
    cmd += " " + args.format(context=context)
    context.dnf["rpmdb_pre"] = get_rpmdb_rpms(context.dnf.installroot)
    run_in_context(context, cmd, can_fail=True)


@behave.given("I do not assume yes")
def given_I_do_not_assumeyes(context):
    context.dnf._set("assumeyes_option", "")


@behave.given("I do not set releasever")
def step_impl(context):
    context.dnf._set("releasever", "")


@behave.given("I do not disable plugins")
def step_impl(context):
    context.dnf._set("disable_plugins", False)


@behave.given("I set dnf command to \"{command}\"")
def step_set_dnf_command(context, command):
    context.dnf.dnf_command = command


@behave.given("I enable plugin \"{plugin}\"")
def given_enable_plugin(context, plugin):
    if "plugins" not in context.dnf:
        context.dnf["plugins"] = []
    if plugin not in context.dnf["plugins"]:
        context.dnf["plugins"].append(plugin)


@behave.given("I successfully execute dnf with args \"{args}\"")
def given_i_successfully_execute_dnf_with_args(context, args):
    context.execute_steps(u"Given I execute dnf with args \"{args}\"".format(args=args))
    assert_exitcode(context, 0)


@behave.given("I successfully execute rpm with args \"{args}\"")
def given_i_successfully_execute_rpm_with_args(context, args):
    context.execute_steps(u"Given I execute rpm with args \"{args}\"".format(args=args))
    assert_exitcode(context, 0)


@behave.step("I set config option \"{option}\" to \"{value}\"")
def step_impl(context, option, value):
    if "setopts" not in context.dnf:
        context.dnf["setopts"] = {}
    context.dnf["setopts"][option] = value


@behave.step('I set up a http server for directory "{path}"')
def step_set_up_http_server(context, path):
    full_path = prepend_installroot(context, path)
    host, port = start_server_based_on_type(context, full_path, 'http')
    context.dnf.ports[path] = port


@behave.then("stdout contains \"{text}\"")
def then_stdout_contains(context, text):
    if re.search(text.format(context=context), context.cmd_stdout):
        return
    raise AssertionError("Stdout doesn't contain: %s" % text)


@behave.then("stdout does not contain \"{text}\"")
def then_stdout_does_not_contain(context, text):
    if not re.search(text.format(context=context), context.cmd_stdout):
        return
    raise AssertionError("Stdout contains: %s" % text)


@behave.then("stdout contains lines")
def then_stdout_contains_lines(context):
    out_lines = [l.strip() for l in context.cmd_stdout.split('\n')]
    test_lines = [l.strip() for l in context.text.split('\n')]
    for line in test_lines:
        for outline in out_lines:
            if line == outline:
                break
        else:
            raise AssertionError("Stdout doesn't contain line: %s" % line)


@behave.then("stdout contains lines matching")
def then_stdout_contains_lines_matching(context):
    out_lines = context.cmd_stdout.split('\n')
    test_lines = context.text.split('\n')
    for line in test_lines:
        for outline in out_lines:
            if re.search("^" + line.strip() + "$", outline.strip()):
                break
        else:
            raise AssertionError("Stderr doesn't contain line matching: %s" % line)


@behave.then("stdout does not contain lines")
def then_stdout_does_not_contain_lines(context):
    out_lines = [l.strip() for l in context.cmd_stdout.split('\n')]
    test_lines = [l.strip() for l in context.text.split('\n')]
    for line in test_lines:
        for outline in out_lines:
            if line == outline:
                raise AssertionError("Stdout contains line: %s" % line)


@behave.then('stdout section "{section}" contains "{regexp}"')
def then_stdout_section_contains(context, section, regexp):
    """Compares the content of a particular section from the command output with a given regexp"""
    section_content = extract_section_content_from_text(section, context.cmd_stdout)
    if re.search(regexp, section_content):
        return
    raise AssertionError("Stdout section %s doesn't contain: %s" % (section, regexp))


@behave.then('stdout section "{section}" does not contain "{regexp}"')
def then_stdout_section_does_not_contain(context, section, regexp):
    section_content = extract_section_content_from_text(section, context.cmd_stdout)
    if re.search(regexp, section_content):
        raise AssertionError("Stdout section %s contains: %s" % (section, regexp))


@behave.then("stderr contains \"{text}\"")
def then_stderr_contains(context, text):
    if re.search(text.format(context=context), context.cmd_stderr):
        return
    raise AssertionError("Stderr doesn't contain: %s" % text)


@behave.then("stderr does not contain \"{text}\"")
def then_stderr_does_not_contain(context, text):
    if not re.search(text.format(context=context), context.cmd_stderr):
        return
    raise AssertionError("Stderr contains: %s" % text)


@behave.then("stderr contains lines")
def then_stderr_contains_lines(context):
    out_lines = context.cmd_stderr.split('\n')
    test_lines = context.text.split('\n')
    for line in test_lines:
        for outline in out_lines:
            if line.strip() == outline.strip():
                break
        else:
            raise AssertionError("Stderr doesn't contain line: %s" % line)


@behave.then("stderr contains lines matching")
def then_stderr_contains_lines_matching(context):
    out_lines = context.cmd_stderr.split('\n')
    test_lines = context.text.split('\n')
    for line in test_lines:
        for outline in out_lines:
            if re.search("^" + line.strip() + "$", outline.strip()):
                break
        else:
            raise AssertionError("Stderr doesn't contain line matching: %s" % line)


@behave.step("I sleep for \"{duration:d}\" seconds")
def step_sleep(context, duration):
    time.sleep(duration)


def execute_python_script(context, script):
    """
    Execute arbitrary python script
    """
    if not sys.executable:
        raise RuntimeError("Python executable not found.")

    cmd = sys.executable + " -c"
    cmd += " \"" + script.format(context=context).replace('"', '\\"') + "\""
    run_in_context(context, cmd, can_fail=True)


@behave.step("I execute python script")
def step_execute_python_script(context):
    """
    Step for executing arbitrary python script
    """
    context.dnf["rpmdb_pre"] = get_rpmdb_rpms(context.dnf.installroot)
    execute_python_script(context, context.text)


@behave.step("I execute python libdnf5 api script with setup")
def execute_python_libdnf5_api_script_with_setup(context):
    """
    Execute snippet of python script using libdnf5 api that is
    appended to already setup base with installroot and loaded
    available repositories.
    """
    context.dnf["rpmdb_pre"] = get_rpmdb_rpms(context.dnf.installroot)

    libdnf5_setup_script = """
import libdnf5
import os

def execute_transaction(goal,description):
    transaction = goal.resolve()
    for tspkg in transaction.get_transaction_packages():
        print(tspkg.get_package().get_nevra(), ":", libdnf5.base.transaction.transaction_item_action_to_string(tspkg.get_action()))
    downloader = libdnf5.repo.PackageDownloader(goal.get_base())
    for tspkg in transaction.get_transaction_packages():
        if libdnf5.base.transaction.transaction_item_action_is_inbound(tspkg.get_action()):
            downloader.add(tspkg.get_package())
    downloader.download()
    transaction.set_description(description)
    transaction.run()

def test_transaction(goal):
    transaction = goal.resolve()
    for tspkg in transaction.get_transaction_packages():
        print(tspkg.get_package().get_nevra(), ":", libdnf5.base.transaction.transaction_item_action_to_string(tspkg.get_action()))
    downloader = libdnf5.repo.PackageDownloader(goal.get_base())
    for tspkg in transaction.get_transaction_packages():
        if libdnf5.base.transaction.transaction_item_action_is_inbound(tspkg.get_action()):
            downloader.add(tspkg.get_package())
    downloader.download()
    return transaction.test()

base = libdnf5.base.Base()
config = base.get_config()
vars = base.get_vars()

config.installroot = '{context.dnf.installroot}'
config.config_file_path = os.path.join(config.installroot, 'etc/dnf/dnf.conf')
config.reposdir = os.path.join(config.installroot, 'etc/yum.repos.d')
config.varsdir = os.path.join(config.installroot, 'etc/dnf/vars')
config.cachedir = os.path.join(config.installroot, 'var/cache/yum')

vars.set('releasever', '{context.dnf.releasever}')
vars.set('basearch', 'x86_64')

base.load_config()

base.setup()

repo_sack = base.get_repo_sack()
repo_sack.create_repos_from_system_configuration()
repo_sack.load_repos()
"""
    execute_python_script(context, libdnf5_setup_script + context.text)


@behave.step("I execute dnf5daemon-client with args \"{args}\"")
def when_I_execute_dnf5daemon_client_with_args(context, args):
    cmd = " ".join(["dnf5daemon-client"])
    cmd += " " + args.format(context=context)
    run_in_context(context, cmd, can_fail=True)


@behave.step("I execute dnf5daemon-client with no args")
def when_I_execute_dnf5daemon_client_with_no_args(context):
    cmd = " ".join(["dnf5daemon-client"])
    run_in_context(context, cmd, can_fail=True)


@behave.step("I remove dnf5daemon-server")
def when_I_remove_dnf5daemon_server(context):
    subprocess.call(["rpm", "-e", "dnf5daemon-server"])


@behave.step("I stop dbus")
def when_I_stop_dbus(context):
    subprocess.call(["killall", "dbus-daemon"])


@behave.step("I stop polkitd")
def when_I_stop_polkitd(context):
    context.p_polkitd.kill()


@behave.step("I stop dnf5daemon-server")
def when_I_stop_dnf5daemon_server(context):
    subprocess.call(["killall", "dnf5daemon-server"])


@behave.then("stdout json matches")
def then_json_matches(context):
    """
    Compare json output from stdout with specified json.
    Specified json can contain fnmatch patterns in string values.
    """
    table_json = json.loads(context.text)
    out_json = json.loads(context.cmd_stdout)
    diffs = diff_json_pattern_values(".", table_json, out_json)
    if diffs:
        for diff in diffs:
            print(diff)
        raise AssertionError("Expected JSON doesn't match")
