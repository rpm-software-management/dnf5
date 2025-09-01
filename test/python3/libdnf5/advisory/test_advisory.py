# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later
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
        query = libdnf5.advisory.AdvisoryQuery(self.base)
        query.filter_type("security")
        self.advisory = next(iter(query))

    def test_advisory_query(self):
        query = libdnf5.advisory.AdvisoryQuery(self.base)
        query.filter_name("DNF-2019-1")
        self.assertEqual(query.size(), 1)
        advisory = next(iter(query))
        self.assertEqual(advisory.get_name(), "DNF-2019-1")

        query = libdnf5.advisory.AdvisoryQuery(self.base)
        query.filter_type("bugfix")
        self.assertEqual(query.size(), 2)
        advisory = next(iter(query))
        self.assertEqual(advisory.get_type(), "bugfix")

        query = libdnf5.advisory.AdvisoryQuery(self.base)
        query.filter_reference("1111")
        self.assertEqual(query.size(), 1)
        advisory = next(iter(query))
        refs = advisory.get_references()
        self.assertEqual(len(refs), 1)
        self.assertEqual(refs[0].get_id(), "1111")

        query = libdnf5.advisory.AdvisoryQuery(self.base)
        query.filter_severity("moderate")
        self.assertEqual(query.size(), 1)
        advisory = next(iter(query))
        self.assertEqual(advisory.get_severity(), "moderate")

        query = libdnf5.advisory.AdvisoryQuery(self.base)
        pkgquery = libdnf5.rpm.PackageQuery(self.base)
        query.filter_packages(pkgquery)
        self.assertEqual(query.size(), 1)
        advisory = next(iter(query))
        self.assertEqual(advisory.get_name(), "DNF-2019-1")

        query = libdnf5.advisory.AdvisoryQuery(self.base)
        pkgquery = libdnf5.rpm.PackageQuery(self.base)
        pkgs = query.get_advisory_packages_sorted(pkgquery)
        self.assertEqual(pkgs.size(), 1)
        self.assertEqual(pkgs[0].get_nevra(), "pkg-1.2-3.x86_64")

    def test_advisory(self):
        self.assertEqual(self.advisory.get_name(), "DNF-2019-1")
        self.assertEqual(self.advisory.get_severity(), "moderate")
        self.assertEqual(self.advisory.get_type(), "security")
        self.assertEqual(self.advisory.get_buildtime(), 1550849401)
        self.assertEqual(self.advisory.get_vendor(), "dnf-testing@redhat.com")
        self.assertEqual(self.advisory.get_description(),
                         "testing advisory 2019")
        self.assertEqual(self.advisory.get_title(), "bugfix_A-1.0-1")
        self.assertEqual(self.advisory.get_status(), "stable")
        self.assertEqual(self.advisory.get_rights(), "")
        self.assertEqual(self.advisory.get_message(), "")
        self.assertEqual(self.advisory.is_applicable(), True)

    def test_references(self):
        refs = self.advisory.get_references()
        self.assertEqual(len(refs), 1)
        reference = refs[0]
        self.assertEqual(reference.get_id(), "1111")
        self.assertEqual(reference.get_type(), "cve")
        self.assertEqual(reference.get_title(), "CVE-2999")
        self.assertEqual(reference.get_url(), "https://foobar/foobarupdate_2")

    def test_collections(self):
        cols = self.advisory.get_collections()
        self.assertEqual(len(cols), 1)
        collection = cols[0]
        self.assertEqual(collection.get_advisory_id(), self.advisory.get_id())
        self.assertEqual(collection.get_advisory(), self.advisory)
        self.assertEqual(collection.is_applicable(), True)

    def test_advisory_packages(self):
        collection = self.advisory.get_collections()[0]
        adv_packages = collection.get_packages()
        self.assertEqual(len(adv_packages), 2)
        adv_package = adv_packages[0]

        self.assertEqual(adv_package.get_advisory_id(), self.advisory.get_id())
        self.assertEqual(adv_package.get_advisory(), self.advisory)
        # We can't compare the collections directly because adv_package constructs
        # a new instance of the same collection
        self.assertEqual(adv_package.get_advisory_collection(
        ).get_advisory_id(), self.advisory.get_id())

        self.assertEqual(adv_package.get_name(), "pkg")
        self.assertEqual(adv_package.get_epoch(), "0")
        self.assertEqual(adv_package.get_version(), "1.2")
        self.assertEqual(adv_package.get_release(), "3")
        self.assertEqual(adv_package.get_evr(), "1.2-3")
        self.assertEqual(adv_package.get_arch(), "x86_64")
        self.assertEqual(adv_package.get_nevra(), "pkg-1.2-3.x86_64")

        self.assertEqual(adv_package.get_reboot_suggested(), True)
        self.assertEqual(adv_package.get_restart_suggested(), False)
        self.assertEqual(adv_package.get_relogin_suggested(), False)

    def test_advisory_modules(self):
        collection = self.advisory.get_collections()[0]
        adv_modules = collection.get_modules()
        self.assertEqual(len(adv_modules), 2)
        adv_module = adv_modules[0]

        self.assertEqual(adv_module.get_advisory_id(), self.advisory.get_id())
        self.assertEqual(adv_module.get_advisory(), self.advisory)
        # We can't compare the collections directly because adv_module constructs
        # a new instance of the same collection
        self.assertEqual(adv_module.get_advisory_collection(
        ).get_advisory_id(), self.advisory.get_id())

        self.assertEqual(adv_module.get_name(), "perl-DBI")
        self.assertEqual(adv_module.get_stream(), "master")
        self.assertEqual(adv_module.get_version(), "2")
        self.assertEqual(adv_module.get_context(), "2a")
        self.assertEqual(adv_module.get_arch(), "x86_64")
        self.assertEqual(adv_module.get_nsvca(), "perl-DBI:master:2:2a:x86_64")
