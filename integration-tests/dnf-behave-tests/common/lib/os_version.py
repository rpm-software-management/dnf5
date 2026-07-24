# -*- coding: utf-8 -*-

import distro
import rpm


def detect_os_version():
    os_id = distro.id()
    major_version = distro.major_version()

    # treat centos as RHEL in context of scenario tag matching
    if os_id == "centos":
        os_id = "rhel"

    # Treat anything that defines non-empty "rhel" RPM macro as RHEL.
    # That's especially needed to handle Fedora ELN as RHEL.
    rhel_macro_version = int(rpm.expandMacro("0%{?rhel}"))
    if rhel_macro_version > 0:
        os_id = "rhel"
        major_version = str(rhel_macro_version)

    return os_id + "__" + major_version

# Whether the current operating systems is supposed to support modularity.


def want_modularity():
    os, version = detect_os_version().split('__')
    if os == 'rhel' and int(version) >= 11:
        return False
    return True
