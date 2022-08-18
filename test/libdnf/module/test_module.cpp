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
#include "libdnf/module/module_item_container.hpp"
#include "libdnf/utils/format.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

CPPUNIT_TEST_SUITE_REGISTRATION(ModuleTest);


using namespace libdnf::module;


void ModuleTest::test_load() {
    std::ifstream module_yaml_file(PROJECT_SOURCE_DIR "/test/data/repos-repomd/repomd-modules/repodata/modules.yaml");
    std::string module_yaml_string;
    std::string line;
    if (module_yaml_file.is_open()) {
        while (getline(module_yaml_file, line)) {
            module_yaml_string.append(line).append("\n");
        }
        module_yaml_file.close();
    }

    auto module_item_container = new ModuleItemContainer(base);
    module_item_container->add(module_yaml_string, "repomd-modules");
    module_item_container->add_modules_without_static_context();
    CPPUNIT_ASSERT_EQUAL(3lu, module_item_container->modules.size());

    auto & meson = module_item_container->modules[0];
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), meson->get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), meson->get_stream());
    CPPUNIT_ASSERT_EQUAL(20180816151613ll, meson->get_version());
    CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), meson->get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), meson->get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("meson:master:20180816151613:06d0a27d:x86_64"), meson->get_full_identifier());
    CPPUNIT_ASSERT_EQUAL(std::string("The Meson Build system"), meson->get_summary());
    CPPUNIT_ASSERT_EQUAL(
        std::string("Meson is an open source build system meant to be both extremely fast, and, even more importantly, "
                    "as user friendly as possible.\nThe main design point of Meson is that every moment a developer "
                    "spends writing or debugging build definitions is a second wasted. So is every second spent "
                    "waiting for the build system to actually start compiling code."),
        meson->get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("ninja;platform:[f29,f30,f31]"), meson->get_module_dependencies_string());

    CPPUNIT_ASSERT_EQUAL(std::string("nodejs"), module_item_container->modules[1]->get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("nodejs"), module_item_container->modules[2]->get_name());
}
