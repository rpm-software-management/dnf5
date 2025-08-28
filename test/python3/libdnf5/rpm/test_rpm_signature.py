# Copyright Contributors to the DNF5 project.
# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

import os

import libdnf5

import base_test_case


class TestRpmSignature(base_test_case.BaseTestCase):
    def setUp(self):
        super().setUp()
        self.add_repo_repomd("repomd-repo1")

    def test_checking_signature(self):
        # Test wrapper for check_package_signature
        query = libdnf5.rpm.PackageQuery(self.base)
        query.filter_name(["pkg"])
        package = next(iter(query))

        rpm_sign = libdnf5.rpm.RpmSignature(self.base)
        result = rpm_sign.check_package_signature(package)
        self.assertEqual(result, libdnf5.rpm.RpmSignature.CheckResult_SKIPPED)

    def test_key_files_vector_wrapper(self):
        # Test wrapper for std::vector<KeyInfo>
        key_path = os.path.join(base_test_case.PROJECT_SOURCE_DIR,
                                "test/data/keys", "key.pub")
        rpm_sign = libdnf5.rpm.RpmSignature(self.base)
        keys = rpm_sign.parse_key_file(key_path)
        _ = [key.get_key_id() for key in keys]
