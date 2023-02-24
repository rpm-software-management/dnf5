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

#include "../shared/utils.hpp"
#include "utils/fs/file.hpp"

#include "libdnf/module/module_errors.hpp"
#include "libdnf/module/module_item.hpp"
#include "libdnf/module/module_query.hpp"
#include "libdnf/module/module_sack.hpp"
#include "libdnf/module/nsvcap.hpp"
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

    ModuleQuery query = ModuleQuery(base, false);
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


void ModuleTest::test_nsvcap() {
    Nsvcap nsvcap;
    CPPUNIT_ASSERT(nsvcap.parse("meson:master:20180816151613:06d0a27d:x86_64/default", Nsvcap::Form::NSVCAP));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master:20180816151613:06d0a27d:x86_64", Nsvcap::Form::NSVCA));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master:20180816151613::x86_64/default", Nsvcap::Form::NSVAP));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master:20180816151613::x86_64", Nsvcap::Form::NSVA));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master::x86_64/default", Nsvcap::Form::NSAP));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master::x86_64", Nsvcap::Form::NSA));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master:20180816151613:06d0a27d/default", Nsvcap::Form::NSVCP));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master:20180816151613:06d0a27d", Nsvcap::Form::NSVC));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master:20180816151613/default", Nsvcap::Form::NSVP));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master:20180816151613", Nsvcap::Form::NSV));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master/default", Nsvcap::Form::NSP));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master", Nsvcap::Form::NS));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson::x86_64/default", Nsvcap::Form::NAP));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson::x86_64", Nsvcap::Form::NA));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson/default", Nsvcap::Form::NP));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson", Nsvcap::Form::N));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master:06d0a27d/default", Nsvcap::Form::NSCP));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), nsvcap.get_profile());

    CPPUNIT_ASSERT(nsvcap.parse("meson:master:06d0a27d", Nsvcap::Form::NSC));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    // There can be additional ':' before architecture even if it's full NSVCA
    CPPUNIT_ASSERT(nsvcap.parse("meson:master:20180816151613:06d0a27d::x86_64/default", Nsvcap::Form::NSVCAP));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), nsvcap.get_profile());
    CPPUNIT_ASSERT(nsvcap.parse("meson:master:20180816151613:06d0a27d::x86_64", Nsvcap::Form::NSVCA));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    // Trailing '/' is allowed
    CPPUNIT_ASSERT(nsvcap.parse("meson/", Nsvcap::Form::N));
    CPPUNIT_ASSERT_EQUAL(std::string("meson"), nsvcap.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_stream());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_version());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_context());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_arch());
    CPPUNIT_ASSERT_EQUAL(std::string(""), nsvcap.get_profile());

    // The form must be matched exactly
    CPPUNIT_ASSERT_EQUAL(false, nsvcap.parse("meson:master", Nsvcap::Form::N));
    CPPUNIT_ASSERT_EQUAL(false, nsvcap.parse("meson:master:20180816151613:06d0a27d/default", Nsvcap::Form::NSVC));
    CPPUNIT_ASSERT_EQUAL(false, nsvcap.parse("meson:master:20180816151613:06d0a27d:", Nsvcap::Form::NSVC));

    // Empty fields are not allowed
    CPPUNIT_ASSERT_EQUAL(false, nsvcap.parse("meson:master::06d0a27d", Nsvcap::Form::NSVC));

    // Strings longer than 2 KB are not allowed
    CPPUNIT_ASSERT_EQUAL(true, nsvcap.parse(std::string(2048, 'a'), Nsvcap::Form::N));
    CPPUNIT_ASSERT_EQUAL(false, nsvcap.parse(std::string(2048 + 1, 'a'), Nsvcap::Form::N));
}


void ModuleTest::test_query_spec() {
    add_repo_repomd("repomd-modules");
    Nsvcap nsvcap;

    {
        ModuleQuery query(base, false);
        nsvcap.parse("meson", Nsvcap::Form::N);
        query.filter_nsvca(nsvcap);
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("meson"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("master"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        ModuleQuery query(base, false);
        nsvcap.parse("gooseberry:5.4", Nsvcap::Form::NS);
        query.filter_nsvca(nsvcap);
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("gooseberry"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("5.4"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("1"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string(""), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        ModuleQuery query(base, false);
        nsvcap.parse("gooseberry:5.5:2", Nsvcap::Form::NSV);
        query.filter_nsvca(nsvcap);
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("gooseberry"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("5.5"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("2"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("72aaf46b6"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        ModuleQuery query(base, false);
        nsvcap.parse("berries:main:4:6c81f848::x86_64", Nsvcap::Form::NSVCA);
        query.filter_nsvca(nsvcap);
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("berries"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("main"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("4"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("6c81f848"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        ModuleQuery query(base, false);
        nsvcap.parse("gooseberry:5.5:2", Nsvcap::Form::NSV);
        query.filter_nsvca(nsvcap);
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("gooseberry"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("5.5"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("2"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("72aaf46b6"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        // NA
        ModuleQuery query(base);
        auto return_value = query.resolve_module_spec("meson::x86_64");
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("meson"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("master"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        // NSVCA glob
        ModuleQuery query(base);
        auto return_value = query.resolve_module_spec("mes*:mas*:2018081615161[13]:*:x8?_64");
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("meson"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("master"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        // NSV
        ModuleQuery query(base);
        auto return_value = query.resolve_module_spec("meson:master:20180816151613");
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("meson"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("master"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        // NSC
        ModuleQuery query(base);
        auto return_value = query.resolve_module_spec("meson:master:06d0a27d");
        CPPUNIT_ASSERT_EQUAL(return_value.first, true);
        auto result = query.get();
        CPPUNIT_ASSERT_EQUAL(std::string("meson"), result.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("master"), result.get_stream());
        CPPUNIT_ASSERT_EQUAL(std::string("20180816151613"), result.get_version_str());
        CPPUNIT_ASSERT_EQUAL(std::string("06d0a27d"), result.get_context());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), result.get_arch());
    }

    {
        // Incorrect spec
        ModuleQuery query(base);
        auto return_value = query.resolve_module_spec("meson/default:x86_64");
        CPPUNIT_ASSERT_EQUAL(return_value.first, false);
        CPPUNIT_ASSERT_EQUAL((size_t)0, query.size());
    }
}
