# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import hashlib


def sha256_checksum(data):
    h = hashlib.new("sha256")
    h.update(data)
    return h.hexdigest()
