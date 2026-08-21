# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import re


from lib.rpm import normalize_epoch
from lib.rpm import RPM


ACTION_RE = re.compile(r"^([^ ].+):$")
PACKAGE_RE = re.compile(r" (?P<name>[^ ]+) *(?P<arch>[^ ]+) *(?P<evr>[^ ]+) *(?P<repo>[^ ]+) *(?P<size>.+)$")
MODULE_LIST_HEADER_RE = re.compile(r"^(Name)\s+(Stream)\s+(Profiles)\s+(Summary)\s*$")
MODULE_STREAM_RE = re.compile(r"(?P<module>[^ ]+) *(?P<stream>[^ ]+)$")
MODULE_STREAM_SWITCH_RE = re.compile(r"(?P<module>[^ ]+) *(?P<stream>[^ ]+ -> [^ ]+)$")

OBSOLETE_REPLACING_LABEL = {
    'en_US': 'replacing',
    'cs_CZ': 'nahrazování',
}
REPLACING_DNF5 = re.compile(
    r"^ +(?P<label>%s) +(?P<name>[^ ]+) +(?P<arch>[^ ]+) +(?P<evr>[^ ]*) +(?P<from_repo>[^ ]*) +(?P<size>.*)$"
    % '|'.join(OBSOLETE_REPLACING_LABEL.values()))


ACTIONS_EN = {
    "Installing": "install",
    "Upgrading": "upgrade",
    "Reinstalling": "reinstall",
    "Installing dependencies": "install-dep",
    "Installing weak dependencies": "install-weak",
    "Removing": "remove",
    "Removing dependent packages": "remove-dep",
    "Removing unused dependencies": "remove-unused",
    "Downgrading": "downgrade",
    "Skipping packages with broken dependencies": "broken",
    "Skipping packages with conflicts": "conflict",
    "Installing group/module packages": "install-group",
    "Installing Groups": "group-install",
    "Installing groups": "group-install",
    "Installing groups dependencies": "group-install",
    "Removing Groups": "group-remove",
    "Removing groups": "group-remove",
    "Upgrading Groups": "group-upgrade",
    "Upgrading groups": "group-upgrade",
    "Installing environmental groups": "env-install",
    "Installing Environment Groups": "env-install",
    "Removing Environment Groups": "env-remove",
    "Removing environmental groups": "env-remove",
    "Upgrading Environment Groups": "env-upgrade",
    "Upgrading environmental groups": "env-upgrade",
    "Installing module profiles": "module-profile-install",
    "Removing module profiles": "module-profile-remove",
    "Disabling module profiles": "module-profile-disable",
    "Enabling module streams": "module-stream-enable",
    "Switching module streams": "module-stream-switch",
    "Disabling modules": "module-disable",
    "Resetting modules": "module-reset",
    "Changing reason": "changing-reason",
    "Changing reason of installed groups": "group-changing-reason",
}


ACTIONS = {}
ACTIONS.update(ACTIONS_EN)


def find_transaction_table_begin_dnf5(context, lines):
    """
    Find a DNF5 transaction table header and return index of the following line:
     Package  Arch  Version  Repository  Size
    """
    trans_start_re = re.compile(r"Package +Arch +Version +Repository +Size")
    for i in range(0, len(lines) - 1):
        if trans_start_re.match(lines[i]):
            return i + 1
    raise RuntimeError("Transaction table start not found")


def find_transaction_table_end_dnf5(context, lines):
    """
    Find a DNF5 transaction table end, an empty line
    """
    for i in range(0, len(lines)):
        if not lines[i].strip():
            # empty line indicates the end of the transaction table
            return i
    raise RuntimeError("Transaction table end not found")


def parse_transaction_table_dnf5(context, lines):
    """
    Find and parse transaction table.
    Return {action: set([rpms])}
    """
    result = {}
    for action in ACTIONS.values():
        result[action] = set()
    result["replaced"] = set()

    table_begin = find_transaction_table_begin_dnf5(context, lines)
    lines = lines[table_begin:]

    table_end = find_transaction_table_end_dnf5(context, lines)
    lines = lines[:table_end]

    while lines:
        line = lines.pop(0).rstrip()

        match = ACTION_RE.match(line)
        if not match:
            raise RuntimeError("Couldn't parse transaction table action: {}".format(line))
        line_action = match.group(1)
        action = ACTIONS[line_action]

        while True:
            if not lines:
                break

            line = lines[0].rstrip()

            if action.startswith('group-') or action.startswith('env-') or action.startswith('module-'):
                if ACTION_RE.match(line):
                    break
                lines.pop(0)
                if '-stream-' in action:
                    if '-switch' in action:
                        match = MODULE_STREAM_SWITCH_RE.match(line.strip())
                        if not match:
                            raise ValueError("Couldn't parse module/stream: {}".format(line))
                    else:
                        match = MODULE_STREAM_RE.match(line.strip())
                        if not match:
                            raise ValueError("Couldn't parse module/stream: {}".format(line))
                    result[action].add("{0[module]}:{0[stream]}".format(match.groupdict()))
                else:
                    group = line.strip()
                    result[action].add(group)
                if action == "group-changing-reason":
                    # Drop line that describes how the reason was changed, eg: User -> Dependency.
                    # Currently we do not record how the reason was changed, it can be checked by
                    # step: "dnf5 transaction items for transaction "last" are"
                    lines.pop(0)
                continue

            result_action = action
            # match the "  replacing ..." line
            match = REPLACING_DNF5.match(line)
            if match:
                result_action = 'replaced'
            else:
                match = PACKAGE_RE.match(line)
            if not match:
                # either next action or parsing error
                break

            lines.pop(0)
            match_dict = match.groupdict()
            match_dict["evr"] = normalize_epoch(match_dict["evr"])
            nevra = "{0[name]}-{0[evr]}.{0[arch]}".format(match_dict)
            rpm = RPM(nevra)
            result[result_action].add(rpm)
            if action == "changing-reason":
                # Drop line that describes how the reason was changed, eg: User -> Dependency.
                # Currently we do not record how the reason was changed, it can be checked by
                # step: "dnf5 transaction items for transaction "last" are"
                lines.pop(0)

    return result


def parse_history_list(history_out):
    lines = history_out.splitlines()
    header = lines[0]
    history_lines = lines[1:]

    # Depending on the data the history list columns have various widths.
    # Load column layout (starting indices) based on the header.
    # The header looks like:
    # "ID Command line    Date and time     Action(s) Altered"
    column_indices = [0]  # ID column starts at first position
    column_indices.append(header.find("Command line"))
    column_indices.append(header.find("Date and time"))
    column_indices.append(header.find("Action"))
    column_indices.append(header.find("Altered"))

    result = []
    labels = ('_line', 'id', 'command', 'date', 'action', 'altered')
    for line in history_lines:
        parts = [line[i:j].strip() for i, j in zip(column_indices, column_indices[1:] + [None])]
        result.append(dict(zip(labels, [line] + parts)))
    return result


def parse_history_info(lines):
    result = dict()
    result[None] = []

    it = iter(lines)
    for line in it:
        if line.lower().startswith("packages altered"):
            next(it)
            for line in it:
                if not line.startswith("  "):
                    break
                result[None].append(line.strip())
            break
        elif ' : ' in line:
            key, val = [s.strip() for s in line.split(' : ', 1)]
            result[key] = val

    return result


def parse_module_list(lines):
    """
    Parse `module list` command output.
    Returns [{'name': module_name, 'stream': module_stream, 'profiles': set([module profiles])}]
    """
    def get_column(idx, columns, line):
        if idx < (len(columns) - 1):
            return line[columns[idx]:columns[idx+1]].strip()
        else:
            return line[columns[idx:]].strip()

    result = []
    columns = []
    header_found = False

    for line in lines:
        # Find header first
        if not header_found:
            match = MODULE_LIST_HEADER_RE.match(line)
            if match:
                columns = [match.start(i + 1) for i in range(4)]
                header_found = True
            continue

        # Empty line separates the module list from the hint line
        if not line.strip():
            break

        module = dict()
        module['name'] = get_column(0, columns, line)
        module['stream'] = get_column(1, columns, line)
        module['profiles'] = set([p.strip() for p in get_column(2, columns, line).split(',')])
        result.append(module)

    return result
