# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave
import re
import parse

from common.lib.cmd import assert_exitcode
from common.lib.diff import print_lines_diff
from common.lib.text import lines_match_to_regexps_line_by_line


sync_line_dnf5 = re.compile(
    r".* [0-9?]+% \| +[0-9.]+ +[KMG]?i?B/s \| +[\-0-9.]+ +[KMG]?i?B \| + [0-9hms?]+")


def strip_reposync_dnf5(found_lines, line_number):
    if line_number < len(found_lines) and found_lines[line_number].strip() == "Updating and loading repositories:":
        found_lines.pop(line_number)

    while line_number < len(found_lines) and sync_line_dnf5.fullmatch(found_lines[line_number].strip()):
        found_lines.pop(line_number)

    if line_number < len(found_lines) and found_lines[line_number].strip() == "Repositories loaded.":
        found_lines.pop(line_number)

    # For command-line repos
    while line_number < len(found_lines) and sync_line_dnf5.fullmatch(found_lines[line_number].strip()):
        found_lines.pop(line_number)


def handle_reposync(expected, found):
    line_number = 0
    for line in expected:
        if line == "<REPOSYNC>":
            strip_reposync_dnf5(found, line_number)

            expected.pop(line_number)
            break

        line_number += 1

    return expected, found


@behave.then("the exit code is {exitcode}")
def then_the_exit_code_is(context, exitcode):
    assert_exitcode(context, exitcode)


@behave.then("stdout is empty")
def then_stdout_is_empty(context):
    if not context.cmd_stdout:
        return
    raise AssertionError("Stdout is not empty, it contains: %s" % context.cmd_stdout)


@behave.then("stdout is")
def then_stdout_is(context):
    """
    Checks for the exact match of the test's stdout. Supports the <REPOSYNC>
    placeholder on the first line, which will match against the repository
    synchronization lines (i.e. the "Last metadata expiration check:" line as
    well as the individual repo download lines) in the test's output.
    """
    expected = context.text.format(context=context).rstrip().split('\n')
    # behave >= 3.0.0 does this rstrip automatically, do it also manually
    # to have the same behavior also with lower versions
    expected = [line.rstrip() for line in expected]
    found = context.cmd_stdout.rstrip().split('\n')
    found = [line.rstrip() for line in found]

    if found == [""]:
        found = []

    clean_expected, clean_found = handle_reposync(expected, found)

    if clean_expected == clean_found:
        return

    rs_offset = 0
    if len(clean_expected) < len(expected):
        if len(clean_found) == len(found):
            rs_offset = 1
            # reposync was not in found, prepend a single line to pad for the
            # <REPOSYNC> line in expected
            found = [""] + found
        else:
            rs_offset = len(found) - len(clean_found)
            # prepend empty lines to expected to pad for multiple reposync
            # lines in found
            expected = [""] * (rs_offset - 1) + expected

    print_lines_diff(expected, found, num_lines_equal=rs_offset)

    raise AssertionError("Stdout is not: %s" % context.text)


@parse.with_pattern(r"stdout|stderr")
def parse_std_stream(text):
    return text


behave.register_type(std_stream=parse_std_stream)


@behave.then("stdout matches line by line")
def then_stdout_matches_line_by_line(context):
    """
    Checks that each line of stdout matches respective line in regular expressions.
    Supports the <REPOSYNC> in the same way as the step "stdout is"
    """
    found = context.cmd_stdout.split('\n')
    expected = context.text.split('\n')

    clean_expected, clean_found = handle_reposync(expected, found)

    lines_match_to_regexps_line_by_line(clean_found, clean_expected)


@behave.then("stderr is empty")
def then_stderr_is_empty(context):
    if not context.cmd_stderr:
        return
    raise AssertionError("Stderr is not empty, it contains: %s" % context.cmd_stderr)


@behave.then("stderr is")
def then_stderr_is(context):
    """
    Checks for the exact match of the test's stderr. Supports the <REPOSYNC>
    placeholder on the first line, which will match against the repository
    synchronization lines (i.e. the "Last metadata expiration check:" line as
    well as the individual repo download lines) in the test's output.
    """
    expected = context.text.format(context=context).strip().split('\n')
    found = context.cmd_stderr.strip().split('\n')

    if found == [""]:
        found = []

    clean_expected, clean_found = handle_reposync(expected, found)

    if clean_expected == clean_found:
        return

    rs_offset = 0
    if len(clean_expected) < len(expected):
        if len(clean_found) == len(found):
            rs_offset = 1
            # reposync was not in found, prepend a single line to pad for the
            # <REPOSYNC> line in expected
            found = [""] + found
        else:
            rs_offset = len(found) - len(clean_found)
            # prepend empty lines to expected to pad for multiple reposync
            # lines in found
            expected = [""] * (rs_offset - 1) + expected

    print_lines_diff(expected, found, num_lines_equal=rs_offset)

    raise AssertionError("Stderr is not: %s" % context.text)


@behave.then("stderr matches line by line")
def then_stderr_matches_line_by_line(context):
    """
    Checks that each line of stderr matches respective line in regular expressions.
    Supports the <REPOSYNC> in the same way as the step "stderr is"
    """
    found = context.cmd_stderr.split('\n')
    expected = context.text.format(context=context).split('\n')

    clean_expected, clean_found = handle_reposync(expected, found)

    lines_match_to_regexps_line_by_line(clean_found, clean_expected)
