# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave
import os
import tomllib

from common.lib.behave_ext import check_context_table
from lib.dnf import parse_module_list


def check_module_list(context):
    check_context_table(context, ["Name", "Stream", "Profiles"])

    lines = context.cmd_stdout.splitlines()
    modules = parse_module_list(lines)

    for name, stream, profiles_txt in context.table:
        if not name or not stream:
            raise ValueError("Invalid row in modules table. Name and stream columns are required")
        if profiles_txt:
            profiles = set([p.strip() for p in profiles_txt.split(',')])
        else:
            profiles = None
        for idx, module in enumerate(modules):
            if module['name'] == name \
               and module['stream'] == stream \
               and (module['profiles'] == profiles if profiles else True):
                modules.pop(idx)
                break
        else:
            raise AssertionError(
                "Module '{}:{}' with profiles '{}' not found.".format(name, stream, profiles_txt))
    return modules


def get_modules_state(installroot):
    found_modules = dict()
    states_filepath = os.path.join(installroot, "usr/lib/sysimage/libdnf5/modules.toml")
    if not os.path.exists(states_filepath):
        return found_modules
    with open(states_filepath, "rb") as f:
        for section_name, section_dict in tomllib.load(f)["modules"].items():
            module_dict = dict()
            if section_dict.get("state"):
                module_dict["state"] = section_dict["state"].lower()
            if section_dict.get("enabled_stream"):
                module_dict["stream"] = section_dict["enabled_stream"]
            if section_dict.get("installed_profiles"):
                module_dict["profiles"] = set(
                    [p.strip() for p in section_dict["installed_profiles"].split(',')])
            found_modules[section_name] = module_dict
    return found_modules


@behave.given("I set default module platformid to \"{platformid}\"")
def step_impl(context, platformid):
    context.dnf._set("module_platform_id", platformid)


@behave.given("I do not set default module platformid")
def given_I_do_not_assumeyes(context):
    context.dnf._set("module_platform_id", "")


@behave.then("module list is empty")
def step_impl(context):
    lines = context.cmd_stdout.splitlines()
    modules = parse_module_list(lines)
    assert not modules, 'Module list is not empty.'


@behave.then("module list contains")
def step_impl(context):
    check_module_list(context)


@behave.then("module list is")
def step_impl(context):
    modules = check_module_list(context)
    if modules:
        remaining_modules = ["{}:{}/{}".format(m['name'], m['stream'], m['profiles'])
                             for m in modules]
        raise AssertionError("Following modules were not captured in the table: %s" % (
            '\n'.join(remaining_modules)))


@behave.then("modules state is following")
def step_impl(context):
    check_context_table(context, ["Module", "State", "Stream", "Profiles"])
    modules_state = get_modules_state(context.dnf.installroot)
    for t_module, t_state, t_stream, t_profiles in context.table:
        module = modules_state.get(t_module, dict())
        state = module.get('state', '')
        if t_state != state and t_state != '?':
            raise AssertionError("Module '{}' state is '{}' and not '{}'".format(
                t_module, state, t_state))
        stream = module.get('stream', '')
        if t_stream != stream and t_stream != '?':
            raise AssertionError("Module '{}' stream is '{}' and not '{}'".format(
                t_module, stream, t_stream))
        profiles = module.get('profiles', set())
        t_profiles = set([p.strip() for p in t_profiles.split(',') if p.strip()])
        if t_profiles != profiles and t_profiles != '?':
            raise AssertionError("Module '{}' profiles are '{}' and not '{}'".format(
                t_module, profiles, t_profiles))
