# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

from behave import fixture
import os


class OSRelease(object):
    """Represents the os-release(5) file."""

    def __init__(self, path):
        self._path = path
        self._backup = None
        # Back up the original file (if any)
        if os.path.exists(path):
            with open(path) as f:
                self._backup = f.read()

    def set(self, data):
        """Store the given data in this file."""
        content = ('%s=%s' % (k, v) for k, v in data.items() if v is not None)
        with open(self._path, 'w') as f:
            f.write('\n'.join(content))

    def delete(self):
        """Delete the file."""
        if os.path.exists(self._path):
            os.remove(self._path)

    def __del__(self):
        """Restore the backup."""
        self.delete()
        if self._backup is not None:
            with open(self._path, 'w') as f:
                f.write(self._backup)


@fixture
def osrelease_fixture(context):
    try:
        if not hasattr(context, "osrelease"):
            path = os.path.realpath('/etc/os-release')
            context.scenario.osrelease = OSRelease(path)

        yield context.scenario.osrelease
    finally:
        del context.scenario.osrelease
