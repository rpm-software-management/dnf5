import unittest

import libdnf5_cli

# See https://discuss.python.org/t/unittest-fail-if-zero-tests-were-discovered/21498, https://github.com/pytest-dev/pytest/issues/2393


class TestPlaceholder(unittest.TestCase):
    def test_placeholder(self):
        pass
