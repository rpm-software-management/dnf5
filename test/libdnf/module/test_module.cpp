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
#include "libdnf/module/module_query.hpp"
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
    CPPUNIT_ASSERT_EQUAL((size_t)10, module_sack->get_modules().size());

    ModuleQuery query = *new ModuleQuery(base, false);
    query.filter_name("meson");
    auto meson = query.get();
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), meson.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), meson.get_stream());
    CPPUNIT_ASSERT_EQUAL(20180816151613ll, meson.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), meson.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), meson.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("meson:master:20180816151613:06d0a27d:x86_64"), meson.get_full_identifier());
    CPPUNIT_ASSERT_EQUAL(std::string("The Meson Build system"), meson.get_summary());
    CPPUNIT_ASSERT_EQUAL(
        std::string("Meson is an open source build system meant to be both extremely fast, and, even more importantly, "
                    "as user friendly as possible.\nThe main design point of Meson is that every moment a developer "
                    "spends writing or debugging build definitions is a second wasted. So is every second spent "
                    "waiting for the build system to actually start compiling code."),
        meson.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("ninja;platform:[f29,f30,f31]"), meson.get_module_dependencies_string());

    CPPUNIT_ASSERT_EQUAL(std::string(""), module_sack->get_default_stream("meson"));
    CPPUNIT_ASSERT_EQUAL(std::string("main"), module_sack->get_default_stream("berries"));
    CPPUNIT_ASSERT_EQUAL((size_t)0, module_sack->get_default_profiles("meson", "master").size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, module_sack->get_default_profiles("berries", "main").size());
    CPPUNIT_ASSERT_EQUAL(std::string("minimal"), module_sack->get_default_profiles("berries", "main")[0]);
}


void ModuleTest::test_resolve() {
    add_repo_repomd("repomd-modules");

    auto module_sack = base.get_module_sack();

    CPPUNIT_ASSERT_EQUAL(ModuleSack::ModuleErrorType::NO_ERROR, module_sack->resolve_active_module_items().second);

    std::vector<std::string> expected_active_module_specs{
        "berries:main:4:6c81f848:x86_64", "gooseberry:5.5:2:72aaf46b6:x86_64", "gooseberry:5.5:3:72aaf46b6:x86_64"};
    std::vector<std::string> active_module_specs;

    for (auto & module_item : module_sack->get_active_modules()) {
        active_module_specs.push_back(module_item->get_full_identifier());
    }
    std::sort(active_module_specs.begin(), active_module_specs.end());
    CPPUNIT_ASSERT_EQUAL(expected_active_module_specs, active_module_specs);
}


void ModuleTest::test_resolve_broken_defaults() {
    add_repo_repomd("repomd-modules-broken-defaults");

    auto module_sack = base.get_module_sack();

    CPPUNIT_ASSERT_EQUAL(
        ModuleSack::ModuleErrorType::ERROR_IN_DEFAULTS, module_sack->resolve_active_module_items().second);

    std::vector<std::string> expected_active_module_specs{
        "berries:main:3:72aaf46b6:x86_64", "gooseberry:5.5:3:72aaf46b6:x86_64"};
    std::vector<std::string> active_module_specs;

    for (auto & module_item : module_sack->get_active_modules()) {
        active_module_specs.push_back(module_item->get_full_identifier());
    }
    std::sort(active_module_specs.begin(), active_module_specs.end());
    CPPUNIT_ASSERT_EQUAL(expected_active_module_specs, active_module_specs);
}


void ModuleTest::test_query() {
    add_repo_repomd("repomd-modules");

    {
        ModuleQuery query(base, false);
        query.filter_name("meson");
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("meson"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("master"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        ModuleQuery query(base, false);
        query.filter_name("gooseberry");
        query.filter_stream("5.4");
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("gooseberry"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("5.4"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("1"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string(""), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        ModuleQuery query(base, false);
        query.filter_name("gooseberry");
        query.filter_stream("5.5");
        query.filter_version("2");
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("gooseberry"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("5.5"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("2"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("72aaf46b6"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        ModuleQuery query(base, false);
        query.filter_name("berries");
        query.filter_context("6c81f848");
        query.filter_arch("x86_64");
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("berries"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("main"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("4"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("6c81f848"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }
}

void ModuleTest::test_query_latest() {
    add_repo_repomd("repomd-modules");
    add_repo_repomd("repomd-modules-duplicit");

    {  // Check we can see all the modules, even ones with duplicit nscva
        ModuleQuery query(base, false);
        CPPUNIT_ASSERT_EQUAL((size_t)13, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_latest(0);
        CPPUNIT_ASSERT_EQUAL((size_t)0, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_latest();
        CPPUNIT_ASSERT_EQUAL((size_t)11, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_latest(-1);
        CPPUNIT_ASSERT_EQUAL((size_t)10, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_name("gooseberry");
        query.filter_latest();
        CPPUNIT_ASSERT_EQUAL((size_t)4, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_name("gooseberry");
        query.filter_latest(2);
        CPPUNIT_ASSERT_EQUAL((size_t)5, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_name("gooseberry");
        query.filter_latest(-1);
        CPPUNIT_ASSERT_EQUAL((size_t)3, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_name("gooseberry");
        query.filter_latest(-2);
        CPPUNIT_ASSERT_EQUAL((size_t)4, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_name("gooseberry");
        query.filter_context("72aaf46b6");
        query.filter_latest();
        for (auto module_item : query) {
            CPPUNIT_ASSERT_EQUAL(std::string("gooseberry"), module_item.get_name());
            CPPUNIT_ASSERT_EQUAL(std::string("5.5"), module_item.get_stream());
            CPPUNIT_ASSERT_EQUAL(std::string("3"), module_item.get_version_str());
            CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), module_item.get_arch());
        }
    }

    {
        ModuleQuery query(base, false);
        query.filter_name("gooseberry");
        query.filter_context("72aaf46b6");
        query.filter_latest(-1);
        for (auto module_item : query) {
            CPPUNIT_ASSERT_EQUAL(std::string("gooseberry"), module_item.get_name());
            CPPUNIT_ASSERT_EQUAL(std::string("5.5"), module_item.get_stream());
            CPPUNIT_ASSERT_EQUAL(std::string("1"), module_item.get_version_str());
            CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), module_item.get_arch());
        }
    }
}
