# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

from behave import fixture
from datetime import datetime
import os


class MachineId(object):
    """Represents the machine-id(5) file."""

    def __init__(self, path):
        self._path = path
        self._backup = path + '.bak'
        if os.path.exists(path):
            os.rename(path, self._backup)

    def _set_mtime(self, value):
        """Set the given mtime on this file."""
        times = None
        if value is not None and value != 'today':
            ts = int(datetime.strptime(value, '%b %d, %Y').timestamp())
            times = (ts, ts)
        os.utime(self._path, times=times)

    def initialize(self, mtime):
        """Initialize the file and set the given mtime."""
        with open(self._path, 'w') as f:
            f.write('dummy\n')
        self._set_mtime(mtime)

    def uninitialize(self, mtime):
        """Uninitialize the file and set the given mtime."""
        with open(self._path, 'w') as f:
            f.write('uninitialized\n')
        self._set_mtime(mtime)

    def empty(self):
        """Empty the file."""
        open(self._path, 'w').close()

    def delete(self):
        """Delete the file."""
        if os.path.exists(self._path):
            os.remove(self._path)

    def __del__(self):
        """Restore the backup."""
        if os.path.exists(self._backup):
            os.rename(self._backup, self._path)


@fixture
def machineid_fixture(context):
    try:
        if not hasattr(context, "machineid"):
            path = os.path.realpath('/etc/machine-id')
            context.scenario.machineid = MachineId(path)

        yield context.scenario.machineid
    finally:
        del context.scenario.machineid
