# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import rpm

from lib.rpm import normalize_epoch
from lib.rpm import RPM


def _str(obj):
    if isinstance(obj, bytes):
        return obj.decode()
    return obj


def get_rpmdb_rpms(installroot="/"):
    """
    Read all installed RPMs from RPM database.
    """
    result = []
    ts = rpm.TransactionSet(installroot)
    for hdr in ts.dbMatch():
        name = _str(hdr["name"])
        evr = _str(hdr["evr"])
        arch = _str(hdr["arch"])
        if name.startswith("gpg-pubkey"):
            continue
        evr = normalize_epoch(evr)
        result.append(RPM("%s-%s.%s" % (name, evr, arch), hdr))
    return result
