# -*- coding: utf-8 -*-

import os

FIXTURES_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "fixtures"))

# If a test is marked with any of these tags, it will be considered
# "destructive" to the system running it.
DESTRUCTIVE_TAGS = [
    "destructive",
    "no_installroot",
]
DNF5DAEMON_TAGS = [
    "dnf5daemon",
]

INVALID_UTF8_CHAR = '\udcfd'
