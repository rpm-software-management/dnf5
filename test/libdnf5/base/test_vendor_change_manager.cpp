// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later


#include "test_vendor_change_manager.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/base/vendor_change_manager.hpp>
#include <libdnf5/base/vendor_change_manager_errors.hpp>
#include <unistd.h>

#include <filesystem>

namespace {

const std::string POLICY_COMPACT_ALLOW_CMDLINE = "@in:[cmdline_repo=\"1\"]";

const std::string POLICY_TOML_ALLOW_CMDLINE =
    "version = '1.2'\n"
    "\n"
    "[[incoming_packages]]\n"
    "filters = [\n"
    "  { filter = 'cmdline_repo', value = \"1\" }\n"
    "]\n";


const std::string POLICY_COMPACT_ALLOW_FEDORA = "in:\"Fedora Project\"";

const std::string POLICY_TOML_ALLOW_FEDORA =
    "version = '1.2'\n"
    "\n"
    "[[incoming_vendors]]\n"
    "vendor = \"Fedora Project\"\n";


const std::string POLICY_COMPACT_TEST_1 = "in:ei*\" test \",eq:^\"MyVendor \",out:\"OldVendor\"";

const std::string POLICY_TOML_TEST_1 =
    "version = '1.2'\n"
    "\n"
    "[[incoming_vendors]]\n"
    "vendor = \" test \"\n"
    "comparator = 'ICONTAINS'\n"
    "exclude = true\n"
    "\n"
    "[[equivalent_vendors]]\n"
    "vendor = \"MyVendor \"\n"
    "comparator = 'STARTSWITH'\n"
    "\n"
    "[[outgoing_vendors]]\n"
    "vendor = \"OldVendor\"\n";


const std::string POLICY_COMPACT_TEST_2 = "in:\"My Vendor\"@in:[source_name=\"mypackage\",version>=\"2.0\"]";

const std::string POLICY_TOML_TEST_2 =
    "version = '1.2'\n"
    "\n"
    "[[incoming_vendors]]\n"
    "vendor = \"My Vendor\"\n"
    "\n"
    "[[incoming_packages]]\n"
    "filters = [\n"
    "  { filter = 'source_name', value = \"mypackage\" },\n"
    "  { filter = 'version', value = \"2.0\", comparator = 'GTE' }\n"
    "]\n";

}  // namespace


CPPUNIT_TEST_SUITE_REGISTRATION(VendorChangeManagerTest);


void VendorChangeManagerTest::setUp() {
    TestCaseFixture::setUp();

    base = get_preconfigured_base();

    const std::filesystem::path installroot = base->get_config().get_installroot_option().get_value();
    distrib_vendor_cfg_dir = installroot / get_install_prefix().relative_path() / "share/dnf5/vendors.d";
    system_vendor_cfg_dir = installroot / "etc/dnf/vendors.d";
    for (const auto & dir : {installroot / "tmp", distrib_vendor_cfg_dir, system_vendor_cfg_dir}) {
        std::filesystem::create_directories(dir);
    }
}


void VendorChangeManagerTest::test_load_policy_from_compact() {
    // Test loading vendor policy from compact format
    //auto base = get_preconfigured_base();
    base->setup();
    auto vcm = base->get_vendor_change_manager();

    // Load compact format
    vcm->load_policy_from_compact(POLICY_COMPACT_TEST_1, "text:test");

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), vcm->get_loaded_policies_count());
    CPPUNIT_ASSERT_EQUAL(std::string("text:test"), vcm->get_loaded_policy_source(0));
    CPPUNIT_ASSERT_EQUAL(POLICY_COMPACT_TEST_1, vcm->get_loaded_policy_as_compact(0));
    CPPUNIT_ASSERT_EQUAL(POLICY_TOML_TEST_1, vcm->get_loaded_policy_as_toml(0));
}


void VendorChangeManagerTest::test_load_policy_from_toml_content() {
    // Test loading vendor policy from TOML content string
    base->setup();
    auto vcm = base->get_vendor_change_manager();

    // Load compact TOML content string
    vcm->load_policy_from_toml(POLICY_TOML_TEST_1, "text:test");

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), vcm->get_loaded_policies_count());
    CPPUNIT_ASSERT_EQUAL(std::string("text:test"), vcm->get_loaded_policy_source(0));
    CPPUNIT_ASSERT_EQUAL(POLICY_COMPACT_TEST_1, vcm->get_loaded_policy_as_compact(0));
    CPPUNIT_ASSERT_EQUAL(POLICY_TOML_TEST_1, vcm->get_loaded_policy_as_toml(0));
}


void VendorChangeManagerTest::test_load_policy_from_toml_file() {
    // Test loading vendor policy from TOML file
    base->setup();
    auto vcm = base->get_vendor_change_manager();

    // Create a TOML file with correct format
    const std::filesystem::path installroot = base->get_config().get_installroot_option().get_value();
    const auto path = installroot / "tmp" / "vendor_policy.conf";
    {
        libdnf5::utils::fs::File(path, "w").write(POLICY_TOML_TEST_1);
    }

    vcm->load_policy_from_toml(std::filesystem::path(path));

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), vcm->get_loaded_policies_count());
    std::string source = vcm->get_loaded_policy_source(0);
    CPPUNIT_ASSERT_EQUAL("file://" + path.string(), source);
    CPPUNIT_ASSERT_EQUAL(POLICY_COMPACT_TEST_1, vcm->get_loaded_policy_as_compact(0));
    CPPUNIT_ASSERT_EQUAL(POLICY_TOML_TEST_1, vcm->get_loaded_policy_as_toml(0));
}


void VendorChangeManagerTest::test_convert_toml_to_compact() {
    // Test converting TOML to compact format
    std::string compact =
        libdnf5::base::VendorChangeManager::convert_policy_toml_to_compact(POLICY_TOML_ALLOW_CMDLINE, "text:test");
    CPPUNIT_ASSERT_EQUAL(POLICY_COMPACT_ALLOW_CMDLINE, compact);

    compact = libdnf5::base::VendorChangeManager::convert_policy_toml_to_compact(POLICY_TOML_ALLOW_FEDORA, "text:test");
    CPPUNIT_ASSERT_EQUAL(POLICY_COMPACT_ALLOW_FEDORA, compact);

    compact = libdnf5::base::VendorChangeManager::convert_policy_toml_to_compact(POLICY_TOML_TEST_1, "text:test");
    CPPUNIT_ASSERT_EQUAL(POLICY_COMPACT_TEST_1, compact);

    compact = libdnf5::base::VendorChangeManager::convert_policy_toml_to_compact(POLICY_TOML_TEST_2, "text:test");
    CPPUNIT_ASSERT_EQUAL(POLICY_COMPACT_TEST_2, compact);
}


void VendorChangeManagerTest::test_convert_toml_file_to_compact() {
    // Test converting TOML file to compact format

    // Create a TOML file with correct format
    const std::filesystem::path installroot = base->get_config().get_installroot_option().get_value();
    const auto path = installroot / "tmp" / "vendor_policy.conf";
    {
        libdnf5::utils::fs::File(path, "w").write(POLICY_TOML_TEST_1);
    }

    std::string compact = libdnf5::base::VendorChangeManager::convert_policy_toml_to_compact(path);
    CPPUNIT_ASSERT_EQUAL(POLICY_COMPACT_TEST_1, compact);
}


void VendorChangeManagerTest::test_convert_compact_to_toml() {
    // Test converting compact format to TOML
    std::string toml =
        libdnf5::base::VendorChangeManager::convert_policy_compact_to_toml(POLICY_COMPACT_ALLOW_CMDLINE, "text:test");
    CPPUNIT_ASSERT_EQUAL(POLICY_TOML_ALLOW_CMDLINE, toml);

    toml = libdnf5::base::VendorChangeManager::convert_policy_compact_to_toml(POLICY_COMPACT_ALLOW_FEDORA, "text:test");
    CPPUNIT_ASSERT_EQUAL(POLICY_TOML_ALLOW_FEDORA, toml);

    toml = libdnf5::base::VendorChangeManager::convert_policy_compact_to_toml(POLICY_COMPACT_TEST_1, "text:test");
    CPPUNIT_ASSERT_EQUAL(POLICY_TOML_TEST_1, toml);

    toml = libdnf5::base::VendorChangeManager::convert_policy_compact_to_toml(POLICY_COMPACT_TEST_2, "text:test");
    CPPUNIT_ASSERT_EQUAL(POLICY_TOML_TEST_2, toml);
}


void VendorChangeManagerTest::test_compact_with_whitespace() {
    // Test parsing compact format with various whitespace patterns
    // Convert to TOML and back, verify result matches normalized compact (without whitespace)

    // Complex pattern with multiple whitespace types
    {
        const std::string compact_mixed =
            "\n  eq :  ^\t \"Fedora\"  ,\t\nin : ei*  \"Vendor\"  @ \n in\n:\t[  arch =\t\"x86_64\" \n ]\n";
        const std::string expected = "eq:^\"Fedora\",in:ei*\"Vendor\"@in:[arch=\"x86_64\"]";

        const std::string toml =
            libdnf5::base::VendorChangeManager::convert_policy_compact_to_toml(compact_mixed, "text:test");
        const std::string result =
            libdnf5::base::VendorChangeManager::convert_policy_toml_to_compact(toml, "text:test");

        CPPUNIT_ASSERT_EQUAL(expected, result);
    }

    // White spaces are not allowed in comparator
    {
        const std::string compact = "in:> =\"Fedora\"";
        CPPUNIT_ASSERT_THROW(
            libdnf5::base::VendorChangeManager::convert_policy_compact_to_toml("in:> =\"Test\"", "text:code"),
            libdnf5::base::VendorChangeManagerError);
        CPPUNIT_ASSERT_THROW(
            libdnf5::base::VendorChangeManager::convert_policy_compact_to_toml("in:! =\"Test\"", "text:code"),
            libdnf5::base::VendorChangeManagerError);
        CPPUNIT_ASSERT_THROW(
            libdnf5::base::VendorChangeManager::convert_policy_compact_to_toml("in:i =\"Test\"", "text:code"),
            libdnf5::base::VendorChangeManagerError);
        CPPUNIT_ASSERT_THROW(
            libdnf5::base::VendorChangeManager::convert_policy_compact_to_toml("in:! i=\"Test\"", "text:code"),
            libdnf5::base::VendorChangeManagerError);
        CPPUNIT_ASSERT_THROW(
            libdnf5::base::VendorChangeManager::convert_policy_compact_to_toml("in:!i =\"Test\"", "text:code"),
            libdnf5::base::VendorChangeManagerError);
    }
}


void VendorChangeManagerTest::test_unload_policies() {
    // Test unload all vendor change policies from memory
    base->setup();
    auto vcm = base->get_vendor_change_manager();

    vcm->load_policy_from_compact("eq:\"V1\", in:\"V2\"", "text:test1");
    vcm->load_policy_from_compact("out:\"V5\", eq:\"V4\"", "text:test2");
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), vcm->get_loaded_policies_count());

    auto unloaded_count = vcm->unload_policies();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), unloaded_count);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), vcm->get_loaded_policies_count());
}


void VendorChangeManagerTest::test_unload_policy() {
    // Test unload a vendor change policy at the specified index
    base->setup();
    auto vcm = base->get_vendor_change_manager();

    vcm->load_policy_from_compact("in:\"V0\"", "text:source0");
    vcm->load_policy_from_compact("in:\"V1\"", "text:source1");
    vcm->load_policy_from_compact("in:\"V2\"", "text:source2");
    vcm->load_policy_from_compact("in:\"V3\"", "text:source3");
    vcm->load_policy_from_compact("in:\"V4\"", "text:source4");
    vcm->load_policy_from_compact("in:\"V5\"", "text:source5");
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(6), vcm->get_loaded_policies_count());

    vcm->unload_policy(2);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(5), vcm->get_loaded_policies_count());
    CPPUNIT_ASSERT_EQUAL(std::string("text:source3"), vcm->get_loaded_policy_source(2));

    vcm->unload_policy(4);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(4), vcm->get_loaded_policies_count());
    CPPUNIT_ASSERT_EQUAL(std::string("text:source4"), vcm->get_loaded_policy_source(3));

    vcm->unload_policy(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), vcm->get_loaded_policies_count());
    CPPUNIT_ASSERT_EQUAL(std::string("text:source1"), vcm->get_loaded_policy_source(0));
    CPPUNIT_ASSERT_EQUAL(std::string("text:source4"), vcm->get_loaded_policy_source(2));

    CPPUNIT_ASSERT_THROW(vcm->unload_policy(999), libdnf5::base::VendorChangeManagerError);
}


void VendorChangeManagerTest::test_unload_policies_matching_source() {
    // Test unload vendor change policies whose source matches the specified pattern
    base->setup();
    auto vcm = base->get_vendor_change_manager();

    vcm->load_policy_from_compact("in:\"V1\"", "text:source1");
    vcm->load_policy_from_compact("in:\"V2\"", "text:source2");
    vcm->load_policy_from_compact("in:\"NV2\"", "text:new_source2");
    vcm->load_policy_from_compact("in:\"V3\"", "text:source3");
    vcm->load_policy_from_compact("in:\"V4\"", "text:source4");
    vcm->load_policy_from_compact("in:\"NV3\"", "text:source3");
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(6), vcm->get_loaded_policies_count());

    auto unloaded_count = vcm->unload_policies_matching_source("text:source3");
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), unloaded_count);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(4), vcm->get_loaded_policies_count());

    unloaded_count = vcm->unload_policies_matching_source("te?t:*source2");
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), unloaded_count);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), vcm->get_loaded_policies_count());
}


void VendorChangeManagerTest::test_extract_policy_base_filename() {
    // Test extracting base filename from policy file path
    std::filesystem::path full_path = "/etc/dnf/vendors.d/my-policy.conf";
    auto base_name = libdnf5::base::VendorChangeManager::extract_policy_base_filename(full_path);

    CPPUNIT_ASSERT_EQUAL(std::string("my-policy"), base_name.string());
}


void VendorChangeManagerTest::test_work_with_files() {
    const std::filesystem::path EMPTY_PATH;

    // Create a TOML file with correct format
    const auto distr_allow_cmdline = distrib_vendor_cfg_dir / "allow_cmdline.conf";
    const auto distr_allow_fedora = distrib_vendor_cfg_dir / "allow_fedora.conf";
    const auto allow_fedora = system_vendor_cfg_dir / "allow_fedora.conf";
    const auto test1 = system_vendor_cfg_dir / "test1.conf";
    {
        libdnf5::utils::fs::File file;
        file.open(distr_allow_cmdline, "w");
        file.write(POLICY_TOML_ALLOW_CMDLINE);
        file.open(distr_allow_fedora, "w");
        file.write(POLICY_TOML_ALLOW_FEDORA);
        file.open(test1, "w");
        file.write(POLICY_TOML_TEST_1);
        file.open(allow_fedora, "w");
        file.write(POLICY_TOML_TEST_2);
    }

    base->setup();
    auto vcm = base->get_vendor_change_manager();

    auto files = vcm->get_policy_files();

    // allow_fedora overides (masks) distr_allow_fedora -> only 3 files loaded
    CPPUNIT_ASSERT_EQUAL(3U, static_cast<unsigned int>(files.size()));
    CPPUNIT_ASSERT_EQUAL(distr_allow_cmdline, files[0]);
    CPPUNIT_ASSERT_EQUAL(allow_fedora, files[1]);
    CPPUNIT_ASSERT_EQUAL(test1, files[2]);

    // Only configuration files in system_vendor_cfg_dir are managable by base::VendorChangeManager
    CPPUNIT_ASSERT(!vcm->is_policy_file_manageable(distr_allow_cmdline));
    CPPUNIT_ASSERT(!vcm->is_policy_file_manageable(distr_allow_fedora));
    CPPUNIT_ASSERT(vcm->is_policy_file_manageable(allow_fedora));
    CPPUNIT_ASSERT(vcm->is_policy_file_manageable(test1));

    // allow_fedora overides (masks) distr_allow_fedora
    CPPUNIT_ASSERT_EQUAL(EMPTY_PATH, vcm->get_masked_policy_file(distr_allow_cmdline));
    CPPUNIT_ASSERT_EQUAL(EMPTY_PATH, vcm->get_masked_policy_file(distr_allow_fedora));
    CPPUNIT_ASSERT_EQUAL(distr_allow_fedora, vcm->get_masked_policy_file(allow_fedora));
    CPPUNIT_ASSERT_EQUAL(EMPTY_PATH, vcm->get_masked_policy_file(test1));

    // Test find_policy_file
    CPPUNIT_ASSERT_EQUAL(distr_allow_cmdline, vcm->find_policy_file("allow_cmdline"));
    CPPUNIT_ASSERT_EQUAL(allow_fedora, vcm->find_policy_file("allow_fedora"));
    CPPUNIT_ASSERT_EQUAL(test1, vcm->find_policy_file("test1"));
    CPPUNIT_ASSERT_EQUAL(EMPTY_PATH, vcm->find_policy_file("non_exist"));

    // Test remove_policy_file
    CPPUNIT_ASSERT_THROW(vcm->remove_policy_file(distr_allow_cmdline), libdnf5::base::VendorChangeManagerError);
    vcm->remove_policy_file("allow_fedora");
    vcm->remove_policy_file("test1");

    files = vcm->get_policy_files();
    CPPUNIT_ASSERT_EQUAL(2U, static_cast<unsigned int>(files.size()));
    CPPUNIT_ASSERT_EQUAL(distr_allow_cmdline, files[0]);
    CPPUNIT_ASSERT_EQUAL(distr_allow_fedora, files[1]);

    // Test save configuration file from string in compact format
    vcm->save_policy_from_compact(POLICY_COMPACT_TEST_2, "test:code", "allow_fedora");
    CPPUNIT_ASSERT_EQUAL(POLICY_TOML_TEST_2, libdnf5::utils::fs::File(allow_fedora, "rb").read());
    // fail - file already exists
    CPPUNIT_ASSERT_THROW(
        vcm->save_policy_from_compact(POLICY_COMPACT_TEST_2, "test:code", "allow_fedora"),
        libdnf5::base::VendorChangeManagerError);

    // Test save configuration file from string in TOML format
    vcm->save_policy_from_toml(POLICY_TOML_TEST_1, "test:code", "test1");
    CPPUNIT_ASSERT_EQUAL(POLICY_TOML_TEST_1, libdnf5::utils::fs::File(test1, "rb").read());

    // Test save configuration file from TOML file
    const std::filesystem::path installroot = base->get_config().get_installroot_option().get_value();
    const auto path = installroot / "tmp" / "toml_test1.conf";
    {
        libdnf5::utils::fs::File(path, "w").write(POLICY_TOML_TEST_1);
    }
    vcm->save_policy_from_toml(path, "from_toml_file");
    CPPUNIT_ASSERT_EQUAL(
        POLICY_TOML_TEST_1, libdnf5::utils::fs::File(system_vendor_cfg_dir / "from_toml_file.conf", "rb").read());

    files = vcm->get_policy_files();
    CPPUNIT_ASSERT_EQUAL(4U, static_cast<unsigned int>(files.size()));
    CPPUNIT_ASSERT_EQUAL(distr_allow_cmdline, files[0]);
    CPPUNIT_ASSERT_EQUAL(allow_fedora, files[1]);
    CPPUNIT_ASSERT_EQUAL(system_vendor_cfg_dir / "from_toml_file.conf", files[2]);
    CPPUNIT_ASSERT_EQUAL(test1, files[3]);
}
