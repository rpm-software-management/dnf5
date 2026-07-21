# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave
import os.path
import re
import shlex

from common.lib.behave_ext import check_context_table
from common.lib.cmd import assert_exitcode, run_in_context
from common.lib.diff import print_lines_diff
from lib.dnf import parse_history_info, parse_history_list


def parsed_history_info(context, spec):
    cmd = " ".join(context.dnf.get_cmd(context) + ["history", "info", spec])
    run_in_context(context, cmd)
    return parse_history_info(context.cmd_stdout.splitlines())


def assert_history_list(context, cmd_stdout):
    def history_equal(history, table):
        if table['Id'] and table['Id'] != history['id']:
            return False
        if table['Action'] and table['Action'] != history['action']:
            return False
        if table['Altered'] and table['Altered'] != history['altered']:
            return False
        if table['Command']:
            # command column in `history list` output is trimmed to limited space
            # to get full command, we need to ask `history info`
            h_info = parsed_history_info(context, history['id'])
            if not table['Command'] in h_info.get('Description', ''):
                return False
        return True

    check_context_table(context, ["Id", "Command", "Action", "Altered"])

    history = parse_history_list(cmd_stdout)

    table_idx = 0
    for t_line in context.table:
        try:
            h_line = history[table_idx]
        except IndexError:
            print(cmd_stdout)
            raise AssertionError(
                "[history] table line (%s, %s, %s, %s) missing in history" % (
                    t_line['Id'], t_line['Command'], t_line['Action'], t_line['Altered']))
        if not history_equal(h_line, t_line):
            print(cmd_stdout)
            raise AssertionError(
                "[history] table line (%s, %s, %s, %s) does not match \"%s\"" % (
                    t_line['Id'], t_line['Command'], t_line['Action'], t_line['Altered'],
                    h_line['_line']))
        table_idx += 1

    if len(history) > table_idx:
        print(cmd_stdout)
        raise AssertionError(
            "[history] Following history lines not captured in the table:\n%s" % (
                '\n'.join(stdout_lines[table_idx:])))


@behave.then('stdout is history list')
def step_impl(context):
    assert_history_list(context, context.cmd_stdout)


@behave.then('History is following')
@behave.then('History "{history_range}" is following')
def step_impl(context, history_range=None):
    if history_range is None:
        history_range = "list"

    cmd = " ".join(context.dnf.get_cmd(context) + ["history", history_range])
    run_in_context(context, cmd)

    assert_history_list(context, context.cmd_stdout)


@behave.then('History info rpmdb version changed')
def step_impl(context, spec=""):
    h_info = parsed_history_info(context, spec)
    assert (h_info['Begin rpmdb']), "End rpmdb version not found"
    assert (h_info['End rpmdb']), "End rpmdb version not found"
    assert (h_info['End rpmdb'] != h_info['Begin rpmdb']), "Begin and end rpmdb versions are the same"


@behave.then('package reasons are')
def step_impl(context):
    check_context_table(context, ["Package", "Reason"])

    cmd = context.dnf.get_cmd(context) + ["repoquery --qf '%{name}-%{evr}.%{arch},%{reason}\n' --installed"]

    run_in_context(context, " ".join(cmd))

    expected = [[p, r] for p, r in context.table]
    found = sorted([r.split(",") for r in context.cmd_stdout.strip().split('\n')])

    if found != expected:
        print_lines_diff(expected, found)
        raise AssertionError("Package reasons mismatch")


# Need to use this complex regex here, as both the first and the third column
# may contain spaces, and a space is also a column separator
transaction_item_re = re.compile("  (.+[^ ]) +(.+-[^ ]+) +(.+[^ ]+) +(.+)")


@behave.then('dnf5 transaction items for transaction "{id}" are')
def step_impl(context, id):
    check_context_table(context, ["action", "package", "reason", "repository"])

    cmd = context.dnf.get_cmd(context) + ["history", "info", id]
    run_in_context(context, " ".join(cmd))

    expected = [(a, p, r, repo) for a, p, r, repo in context.table]
    found = []
    parse = False
    for line in context.cmd_stdout.strip().split('\n'):
        if not parse:
            if line.split() == ["Action", "Package", "Reason", "Repository"]:
                parse = True
            else:
                continue
        else:
            res = transaction_item_re.match(line)
            if res is not None:
                found.append(res.groups())

    if found != expected:
        print_lines_diff(expected, found)
        raise AssertionError("Package reasons mismatch")


@behave.given("I adjust history database with query")
def step_impl(context):
    history_db_location = os.path.join(context.dnf.installroot, "usr/lib/sysimage/libdnf5/transaction_history.sqlite")
    cmd_list = ["sqlite3", history_db_location, context.text]
    cmd_string = " ".join(shlex.quote(s) for s in cmd_list)
    run_in_context(context, cmd_string)
    assert_exitcode(context, 0)
