# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import os

from common.lib.file import create_file_with_contents, ensure_directory_exists


def write_config(context):
    config_dir = os.path.join(context.dnf.installroot, "etc/dnf")
    ensure_directory_exists(config_dir)

    conf_text = ""
    # sort and put [main] first
    for section, values in \
            sorted(list(context.dnf.config.items()), key=lambda i: "" if i[0] == "[main]" else i[0]):
        conf_text += "%s\n" % section
        for k, v in values.items():
            if v != "":
                conf_text += "%s=%s\n" % (k, v.format(context=context))

    create_file_with_contents(os.path.join(config_dir, "dnf.conf"), conf_text)
