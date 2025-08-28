# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

import base_test_case


class TestVars(base_test_case.BaseTestCase):
    def test_getting_undefined_variable(self):
        vars = self.base.get_vars()
        self.assertRaises(IndexError, vars.get_value, "undefined")
