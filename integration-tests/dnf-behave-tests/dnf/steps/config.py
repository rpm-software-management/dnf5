# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave

from common.lib.behave_ext import check_context_table
from lib.config import write_config


@behave.step("I configure dnf with")
def step_configure_dnf(context):
    """
    Merges the new configuration values with what is stored in DNFContext and
    writes the config file at <installroot>/etc/dnf/dnf.conf.

    [main] is the default section, you can add more sections like so:

    Given I configure dnf with
          | key          | value |
          | best         | False |
          | [my_section] |       |
          | foo          | bar   |
    """
    check_context_table(context, ["key", "value"])
    section = "[main]"
    context.dnf.config.setdefault(section, {})
    for k, v in context.table:
        if k.startswith("[") and v == "":
            section = k
            context.dnf.config.setdefault(section, {})

        context.dnf.config[section][k] = v

    write_config(context)
