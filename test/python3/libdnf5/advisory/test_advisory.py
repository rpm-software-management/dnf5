# Copyright Contributors to the libdnf project.
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

import libdnf5

import base_test_case


class TestAdvisory(base_test_case.BaseTestCase):
    def setUp(self):
        super().setUp()
        self.add_repo_repomd("repomd-repo1")

    def test_get_name(self):
        query = libdnf5.advisory.AdvisoryQuery(self.base)
        query.filter_type("security")
        pkg = next(iter(query))
        self.assertEqual(pkg.get_name(), "DNF-2019-1")
