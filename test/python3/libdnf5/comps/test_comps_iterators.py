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


class TestCompsIterators(base_test_case.BaseTestCase):
    def test_group_query_iterable(self):
        self.add_repo_repomd("repomd-comps-core")
        query = libdnf5.comps.GroupQuery(self.base)
        self.assertIsInstance(query.get_base().get(), libdnf5.base.Base)
        assert [grp.get_groupid() for grp in query]
        self.assertIsInstance(
            next(iter(query)).get_base().get(), libdnf5.base.Base)

    def test_environment_query_iterable(self):
        self.add_repo_repomd("repomd-comps-core-environment")
        query = libdnf5.comps.EnvironmentQuery(self.base)
        self.assertIsInstance(query.get_base().get(), libdnf5.base.Base)
        assert [env.get_environmentid() for env in query]
        self.assertIsInstance(
            next(iter(query)).get_base().get(), libdnf5.base.Base)
