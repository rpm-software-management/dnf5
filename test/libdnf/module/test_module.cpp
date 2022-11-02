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


#include "test_module.hpp"

#include "utils.hpp"
#include "utils/fs/file.hpp"

#include "libdnf/module/module_errors.hpp"
#include "libdnf/module/module_item.hpp"
#include "libdnf/module/module_sack.hpp"
#include "libdnf/utils/format.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

CPPUNIT_TEST_SUITE_REGISTRATION(ModuleTest);


using namespace libdnf::module;


void ModuleTest::test_load() {
    add_repo_repomd("repomd-modules");

    auto module_sack = base.get_module_sack();
    CPPUNIT_ASSERT_EQUAL((size_t)9, module_sack->get_modules().size());

    // TODO(pkratoch): Change this once individual modules can be queried
    bool meson_checked = false;
    for (auto & module_item : module_sack->get_modules()) {
        if (module_item->get_name() != "meson") {
            continue;
        }
        CPPUNIT_ASSERT_EQUAL(std::string("meson"), module_item->get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("master"), module_item->get_stream());
        CPPUNIT_ASSERT_EQUAL(20180816151613ll, module_item->get_version());
        CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), module_item->get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), module_item->get_arch());
        CPPUNIT_ASSERT_EQUAL(
            std::string("meson:master:20180816151613:06d0a27d:x86_64"), module_item->get_full_identifier());
        CPPUNIT_ASSERT_EQUAL(std::string("The Meson Build system"), module_item->get_summary());
        CPPUNIT_ASSERT_EQUAL(
            std::string(
                "Meson is an open source build system meant to be both extremely fast, and, even more importantly, "
                "as user friendly as possible.\nThe main design point of Meson is that every moment a developer "
                "spends writing or debugging build definitions is a second wasted. So is every second spent "
                "waiting for the build system to actually start compiling code."),
            module_item->get_description());
        CPPUNIT_ASSERT_EQUAL(
            std::string("ninja;platform:[f29,f30,f31]"), module_item->get_module_dependencies_string());
        meson_checked = true;
    }
    CPPUNIT_ASSERT_EQUAL(true, meson_checked);

    CPPUNIT_ASSERT_EQUAL(std::string(""), module_sack->get_default_stream("meson"));
    CPPUNIT_ASSERT_EQUAL(std::string("main"), module_sack->get_default_stream("berries"));
    CPPUNIT_ASSERT_EQUAL((size_t)0, module_sack->get_default_profiles("meson", "master").size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, module_sack->get_default_profiles("berries", "main").size());
    CPPUNIT_ASSERT_EQUAL(std::string("minimal"), module_sack->get_default_profiles("berries", "main")[0]);
}
