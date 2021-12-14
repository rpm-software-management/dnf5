/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "test_group.hpp"

#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/group/package.hpp"
#include "libdnf/comps/group/query.hpp"

#include <filesystem>
#include <fstream>


CPPUNIT_TEST_SUITE_REGISTRATION(CompsGroupTest);


void CompsGroupTest::test_load() {
    add_repo_repomd("repomd-comps-core");

    libdnf::comps::GroupQuery q_core(base.get_comps()->get_group_sack());
    q_core.filter_installed(false);
    q_core.filter_groupid("core");
    auto core = q_core.get();
    CPPUNIT_ASSERT_EQUAL(std::string("core"), core.get_groupid());
    CPPUNIT_ASSERT_EQUAL(std::string("Core"), core.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Kern"), core.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Smallest possible installation"), core.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Kleinstmögliche Installation"), core.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("1"), core.get_order());
    CPPUNIT_ASSERT_EQUAL(std::string("it"), core.get_langonly());
    CPPUNIT_ASSERT_EQUAL(false, core.get_uservisible());
    CPPUNIT_ASSERT_EQUAL(false, core.get_default());
    CPPUNIT_ASSERT_EQUAL(false, core.get_installed());
    CPPUNIT_ASSERT_EQUAL(5lu, core.get_packages().size());
    std::vector<libdnf::comps::Package> exp_pkgs_core;
    exp_pkgs_core.push_back(libdnf::comps::Package("bash", libdnf::comps::PackageType::MANDATORY, ""));
    exp_pkgs_core.push_back(libdnf::comps::Package("glibc", libdnf::comps::PackageType::MANDATORY, ""));
    exp_pkgs_core.push_back(libdnf::comps::Package("dnf", libdnf::comps::PackageType::DEFAULT, ""));
    exp_pkgs_core.push_back(
        libdnf::comps::Package("conditional", libdnf::comps::PackageType::CONDITIONAL, "nonexistent"));
    exp_pkgs_core.push_back(libdnf::comps::Package("dnf-plugins-core", libdnf::comps::PackageType::OPTIONAL, ""));
    for (unsigned i = 0; i < exp_pkgs_core.size(); i++) {
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_core[i].get_name(), core.get_packages()[i].get_name());
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_core[i].get_type(), core.get_packages()[i].get_type());
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_core[i].get_condition(), core.get_packages()[i].get_condition());
    }

    add_repo_repomd("repomd-comps-standard");

    libdnf::comps::GroupQuery q_standard(base.get_comps()->get_group_sack());
    q_standard.filter_installed(false);
    q_standard.filter_groupid("standard");
    auto standard = q_standard.get();
    CPPUNIT_ASSERT_EQUAL(std::string("standard"), standard.get_groupid());
    CPPUNIT_ASSERT_EQUAL(std::string("Standard"), standard.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("標準"), standard.get_translated_name("ja"));
    CPPUNIT_ASSERT_EQUAL(
        std::string("Common set of utilities that extend the minimal installation."), standard.get_description());
    CPPUNIT_ASSERT_EQUAL(
        std::string("最小限のインストールを拡張するユーティリティの共通セット"),
        standard.get_translated_description("ja"));
    CPPUNIT_ASSERT_EQUAL(std::string("1"), standard.get_order());
    CPPUNIT_ASSERT_EQUAL(std::string(""), standard.get_langonly());
    CPPUNIT_ASSERT_EQUAL(false, standard.get_uservisible());
    CPPUNIT_ASSERT_EQUAL(false, standard.get_default());
    CPPUNIT_ASSERT_EQUAL(false, standard.get_installed());
    CPPUNIT_ASSERT_EQUAL(3lu, standard.get_packages().size());
    std::vector<libdnf::comps::Package> exp_pkgs_standard;
    exp_pkgs_standard.push_back(libdnf::comps::Package("cryptsetup", libdnf::comps::PackageType::MANDATORY, ""));
    exp_pkgs_standard.push_back(
        libdnf::comps::Package("chrony", libdnf::comps::PackageType::CONDITIONAL, "gnome-control-center"));
    exp_pkgs_standard.push_back(
        libdnf::comps::Package("conditional", libdnf::comps::PackageType::CONDITIONAL, "nonexistent"));
    for (unsigned i = 0; i < exp_pkgs_standard.size(); i++) {
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_standard[i].get_name(), standard.get_packages()[i].get_name());
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_standard[i].get_type(), standard.get_packages()[i].get_type());
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_standard[i].get_condition(), standard.get_packages()[i].get_condition());
    }
}


void CompsGroupTest::test_load_defaults() {
    add_repo_repomd("repomd-comps-core-empty");

    libdnf::comps::GroupQuery q_core_empty(base.get_comps()->get_group_sack());
    q_core_empty.filter_groupid("core");
    auto core_empty = q_core_empty.get();
    CPPUNIT_ASSERT_EQUAL(std::string("core"), core_empty.get_groupid());
    CPPUNIT_ASSERT_EQUAL(std::string(""), core_empty.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string(""), core_empty.get_translated_name("ja"));
    CPPUNIT_ASSERT_EQUAL(std::string(""), core_empty.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string(""), core_empty.get_translated_description("ja"));
    CPPUNIT_ASSERT_EQUAL(std::string(""), core_empty.get_order());
    CPPUNIT_ASSERT_EQUAL(std::string(""), core_empty.get_langonly());
    CPPUNIT_ASSERT_EQUAL(true, core_empty.get_uservisible());
    CPPUNIT_ASSERT_EQUAL(false, core_empty.get_default());
    CPPUNIT_ASSERT_EQUAL(false, core_empty.get_installed());
    CPPUNIT_ASSERT_EQUAL(0lu, core_empty.get_packages().size());
}


void CompsGroupTest::test_merge() {
    add_repo_repomd("repomd-comps-core");
    add_repo_repomd("repomd-comps-standard");
    // load another definiton of the core group that changes all attributes
    add_repo_repomd("repomd-comps-core-v2");

    libdnf::comps::GroupQuery q_core2(base.get_comps()->get_group_sack());
    q_core2.filter_groupid("core");
    auto core2 = q_core2.get();
    CPPUNIT_ASSERT_EQUAL(std::string("core"), core2.get_groupid());
    CPPUNIT_ASSERT_EQUAL(std::string("Core v2"), core2.get_name());
    // When attributes are missing in core-v2.xml, original values are kept
    CPPUNIT_ASSERT_EQUAL(std::string("Kern v2"), core2.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Smallest possible installation v2"), core2.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Kleinstmögliche Installation v2"), core2.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("2"), core2.get_order());
    CPPUNIT_ASSERT_EQUAL(std::string("de"), core2.get_langonly());
    // When boolean attributes are missing in core-v2.xml, default values are taken
    CPPUNIT_ASSERT_EQUAL(true, core2.get_uservisible());
    CPPUNIT_ASSERT_EQUAL(true, core2.get_default());
    CPPUNIT_ASSERT_EQUAL(false, core2.get_installed());
    CPPUNIT_ASSERT_EQUAL(4lu, core2.get_packages().size());
    std::vector<libdnf::comps::Package> exp_pkgs_core2;
    exp_pkgs_core2.push_back(libdnf::comps::Package("bash", libdnf::comps::PackageType::MANDATORY, ""));
    exp_pkgs_core2.push_back(libdnf::comps::Package("glibc", libdnf::comps::PackageType::MANDATORY, ""));
    exp_pkgs_core2.push_back(libdnf::comps::Package("dnf", libdnf::comps::PackageType::DEFAULT, ""));
    exp_pkgs_core2.push_back(libdnf::comps::Package("dnf-plugins-core", libdnf::comps::PackageType::OPTIONAL, ""));
    for (unsigned i = 0; i < exp_pkgs_core2.size(); i++) {
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_core2[i].get_name(), core2.get_packages()[i].get_name());
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_core2[i].get_type(), core2.get_packages()[i].get_type());
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_core2[i].get_condition(), core2.get_packages()[i].get_condition());
    }
}


void CompsGroupTest::test_merge_with_empty() {
    add_repo_repomd("repomd-comps-core");
    add_repo_repomd("repomd-comps-standard");
    // load another definiton of the core group that has all attributes empty
    add_repo_repomd("repomd-comps-core-empty");

    libdnf::comps::GroupQuery q_core_empty(base.get_comps()->get_group_sack());
    q_core_empty.filter_groupid("core");
    auto core_empty = q_core_empty.get();
    CPPUNIT_ASSERT_EQUAL(std::string("core"), core_empty.get_groupid());
    CPPUNIT_ASSERT_EQUAL(std::string("Core"), core_empty.get_name());
    // attributes are missing in core-empty.xml -> original values are kept
    CPPUNIT_ASSERT_EQUAL(std::string("Kern"), core_empty.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Smallest possible installation"), core_empty.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Kleinstmögliche Installation"), core_empty.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("1"), core_empty.get_order());
    CPPUNIT_ASSERT_EQUAL(std::string("it"), core_empty.get_langonly());
    // boolean attributes are missing in core-empty.xml -> default values are taken
    CPPUNIT_ASSERT_EQUAL(true, core_empty.get_uservisible());
    CPPUNIT_ASSERT_EQUAL(false, core_empty.get_default());
    CPPUNIT_ASSERT_EQUAL(false, core_empty.get_installed());
    CPPUNIT_ASSERT_EQUAL(0lu, core_empty.get_packages().size());
}


void CompsGroupTest::test_merge_empty_with_nonempty() {
    // load definiton of the core group that has all attributes empty
    add_repo_repomd("repomd-comps-core-empty");
    add_repo_repomd("repomd-comps-standard");
    // load another definiton of the core group that has all attributes filled
    add_repo_repomd("repomd-comps-core");

    libdnf::comps::GroupQuery q_core(base.get_comps()->get_group_sack());
    q_core.filter_groupid("core");
    auto core = q_core.get();
    CPPUNIT_ASSERT_EQUAL(std::string("core"), core.get_groupid());
    CPPUNIT_ASSERT_EQUAL(std::string("Core"), core.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Kern"), core.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Smallest possible installation"), core.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Kleinstmögliche Installation"), core.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("1"), core.get_order());
    CPPUNIT_ASSERT_EQUAL(std::string("it"), core.get_langonly());
    CPPUNIT_ASSERT_EQUAL(false, core.get_uservisible());
    CPPUNIT_ASSERT_EQUAL(false, core.get_default());
    CPPUNIT_ASSERT_EQUAL(false, core.get_installed());
    CPPUNIT_ASSERT_EQUAL(5lu, core.get_packages().size());
    std::vector<libdnf::comps::Package> exp_pkgs_core;
    exp_pkgs_core.push_back(libdnf::comps::Package("bash", libdnf::comps::PackageType::MANDATORY, ""));
    exp_pkgs_core.push_back(libdnf::comps::Package("glibc", libdnf::comps::PackageType::MANDATORY, ""));
    exp_pkgs_core.push_back(libdnf::comps::Package("dnf", libdnf::comps::PackageType::DEFAULT, ""));
    exp_pkgs_core.push_back(
        libdnf::comps::Package("conditional", libdnf::comps::PackageType::CONDITIONAL, "nonexistent"));
    exp_pkgs_core.push_back(libdnf::comps::Package("dnf-plugins-core", libdnf::comps::PackageType::OPTIONAL, ""));
    for (unsigned i = 0; i < exp_pkgs_core.size(); i++) {
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_core[i].get_name(), core.get_packages()[i].get_name());
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_core[i].get_type(), core.get_packages()[i].get_type());
        CPPUNIT_ASSERT_EQUAL(exp_pkgs_core[i].get_condition(), core.get_packages()[i].get_condition());
    }
}


void CompsGroupTest::test_dump() {
    add_repo_repomd("repomd-comps-standard");

    libdnf::comps::GroupQuery q_standard(base.get_comps()->get_group_sack());
    q_standard.filter_groupid("standard");
    auto standard = q_standard.get();

    auto dump_path = std::filesystem::temp_directory_path() / "dumped-standard.xml";
    standard.dump(dump_path);

    std::ifstream dumped_stream(dump_path);
    std::string actual;
    actual.assign(std::istreambuf_iterator<char>(dumped_stream), std::istreambuf_iterator<char>());

    std::filesystem::path expected_path =
        PROJECT_SOURCE_DIR "/test/data/repos-repomd/repomd-comps-standard/repodata/comps.xml";
    std::ifstream expected_stream(expected_path);
    std::string expected;
    expected.assign(std::istreambuf_iterator<char>(expected_stream), std::istreambuf_iterator<char>());

    CPPUNIT_ASSERT_EQUAL(expected, actual);
}
