// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifdef WITH_MODULEMD


#include "test_module.hpp"

#include "../shared/private_accessor.hpp"
#include "../shared/utils.hpp"
#include "base/base_impl.hpp"
#include "module/module_db.hpp"
#include "system/state.hpp"

#include <libdnf5/base/goal.hpp>
#include <libdnf5/base/goal_elements.hpp>
#include <libdnf5/module/module_errors.hpp>
#include <libdnf5/module/module_item.hpp>
#include <libdnf5/module/module_query.hpp>
#include <libdnf5/module/module_sack.hpp>
#include <libdnf5/module/nsvcap.hpp>
#include <libdnf5/utils/format.hpp>

#include <string>

CPPUNIT_TEST_SUITE_REGISTRATION(ModuleTest);


namespace {

// Accessor of private Base::p_impl, see private_accessor.hpp
create_private_getter_template;
create_getter(priv_impl, &libdnf5::Base::p_impl);

}  // namespace


using namespace libdnf5::module;


void ModuleTest::test_load() {
    add_repo_repomd("repomd-modules");

    auto module_sack = base.get_module_sack();
    CPPUNIT_ASSERT_EQUAL((size_t)12, module_sack->get_modules().size());

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
    CPPUNIT_ASSERT_EQUAL(std::string("ninja:[];platform:[f29,f30,f31]"), meson.get_module_dependencies_string());
    CPPUNIT_ASSERT_EQUAL((size_t)1, meson.get_profiles().size());
    CPPUNIT_ASSERT_EQUAL(std::string("default"), meson.get_profiles()[0].get_name());
    CPPUNIT_ASSERT_EQUAL(false, meson.get_profiles()[0].is_default());

    CPPUNIT_ASSERT_EQUAL(std::string(""), module_sack->get_default_stream("meson"));
    CPPUNIT_ASSERT_EQUAL(std::string("main"), module_sack->get_default_stream("berries"));
    CPPUNIT_ASSERT_EQUAL((size_t)0, module_sack->get_default_profiles("meson", "master").size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, module_sack->get_default_profiles("berries", "main").size());
    CPPUNIT_ASSERT_EQUAL(std::string("minimal"), module_sack->get_default_profiles("berries", "main")[0]);

    ModuleQuery query_berries = ModuleQuery(base, false);
    query_berries.filter_name("berries");
    query_berries.filter_stream("main");
    for (const auto & berries : query_berries.list()) {
        for (const auto & berries_profile : berries.get_profiles()) {
            CPPUNIT_ASSERT_EQUAL(true, berries_profile.is_default());
        }
    }
}


void ModuleTest::test_resolve() {
    add_repo_repomd("repomd-modules");

    auto module_sack = base.get_module_sack();

    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalProblem::NO_PROBLEM, module_sack->resolve_active_module_items().second);

    std::vector<std::string> expected_active_module_specs{
        "NoStaticContext:latest:1::x86_64",
        "berries:main:4:6c81f848:x86_64",
        "gooseberry:5.5:2:72aaf46b6:x86_64",
        "gooseberry:5.5:3:72aaf46b6:x86_64"};
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
        libdnf5::GoalProblem::MODULE_SOLVER_ERROR_DEFAULTS, module_sack->resolve_active_module_items().second);

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
    add_repo_repomd("repomd-modules", false);
    add_repo_repomd("repomd-modules-duplicit");

    {  // Check we can see all the modules, even ones with duplicit nscva
        ModuleQuery query(base, false);
        CPPUNIT_ASSERT_EQUAL((size_t)15, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_latest(0);
        CPPUNIT_ASSERT_EQUAL((size_t)0, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_latest();
        CPPUNIT_ASSERT_EQUAL((size_t)13, query.size());
    }

    {
        ModuleQuery query(base, false);
        query.filter_latest(-1);
        CPPUNIT_ASSERT_EQUAL((size_t)12, query.size());
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


void ModuleTest::test_module_db() {
    add_repo_repomd("repomd-modules");

    ModuleDB module_db = ModuleDB(base.get_weak_ptr());
    module_db.initialize();

    // Check initial state of a module that wasn't in the `system::state`
    CPPUNIT_ASSERT_EQUAL(ModuleStatus::AVAILABLE, module_db.get_status("meson"));
    CPPUNIT_ASSERT_EQUAL(std::string(""), module_db.get_enabled_stream("meson"));
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), module_db.get_installed_profiles("meson"));

    CPPUNIT_ASSERT(module_db.get_all_newly_disabled_modules().empty());
    CPPUNIT_ASSERT(module_db.get_all_newly_reset_modules().empty());
    CPPUNIT_ASSERT(module_db.get_all_newly_enabled_streams().empty());
    CPPUNIT_ASSERT(module_db.get_all_newly_disabled_streams().empty());
    CPPUNIT_ASSERT(module_db.get_all_newly_reset_streams().empty());
    CPPUNIT_ASSERT(module_db.get_all_newly_switched_streams().empty());
    CPPUNIT_ASSERT(module_db.get_all_newly_installed_profiles().empty());
    CPPUNIT_ASSERT(module_db.get_all_newly_removed_profiles().empty());

    // Check state of a module after modification
    CPPUNIT_ASSERT(module_db.change_status("meson", ModuleStatus::ENABLED));
    CPPUNIT_ASSERT(module_db.change_stream("meson", "master"));
    CPPUNIT_ASSERT(module_db.add_profile("meson", "default"));
    CPPUNIT_ASSERT_EQUAL(ModuleStatus::ENABLED, module_db.get_status("meson"));
    CPPUNIT_ASSERT_EQUAL(std::string("master"), module_db.get_enabled_stream("meson"));
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>({"default"}), module_db.get_installed_profiles("meson"));

    CPPUNIT_ASSERT(module_db.get_all_newly_disabled_modules().empty());
    CPPUNIT_ASSERT(module_db.get_all_newly_reset_modules().empty());
    CPPUNIT_ASSERT_EQUAL((size_t)1, module_db.get_all_newly_enabled_streams().size());
    CPPUNIT_ASSERT_EQUAL(std::string("master"), module_db.get_all_newly_enabled_streams()["meson"]);
    CPPUNIT_ASSERT(module_db.get_all_newly_disabled_streams().empty());
    CPPUNIT_ASSERT(module_db.get_all_newly_reset_streams().empty());
    CPPUNIT_ASSERT(module_db.get_all_newly_switched_streams().empty());
    CPPUNIT_ASSERT_EQUAL((size_t)1, module_db.get_all_newly_installed_profiles().size());
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>({"default"}), module_db.get_all_newly_installed_profiles()["meson"]);
    CPPUNIT_ASSERT(module_db.get_all_newly_removed_profiles().empty());
}


void ModuleTest::test_module_enable() {
    add_repo_repomd("repomd-modules");

    // Add module enable goal operation
    libdnf5::Goal goal(base);
    goal.add_module_enable("fruit-salad:main", libdnf5::GoalJobSettings());
    auto transaction = goal.resolve();

    // Active modules contain the enabled fruit-salad, its dependency gooseberry and the default streams of modules
    // NoStaticContext and berries
    std::vector<std::string> expected_active_module_specs{
        "NoStaticContext:latest:1::x86_64",
        "berries:main:4:6c81f848:x86_64",
        "fruit-salad:main:12:2241675a:x86_64",
        "gooseberry:5.5:2:72aaf46b6:x86_64",
        "gooseberry:5.5:3:72aaf46b6:x86_64"};
    std::vector<std::string> active_module_specs;
    for (auto & module_item : base.get_module_sack()->get_active_modules()) {
        active_module_specs.push_back(module_item->get_full_identifier());
    }
    std::sort(active_module_specs.begin(), active_module_specs.end());
    CPPUNIT_ASSERT_EQUAL(expected_active_module_specs, active_module_specs);

    // Run the transaction
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::Transaction::TransactionRunResult::SUCCESS, transaction.run());

    auto system_state = (base.*get(priv_impl()))->get_system_state();

    // Module fruit-salat is ENABLED because it was explicitly enabled
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::system::ModuleState({"main", ModuleStatus::ENABLED, {}}),
        system_state.get_module_state("fruit-salad"));
    // Module goosebery is ENABLED because module fruit-salad requires it
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::system::ModuleState({"5.5", ModuleStatus::ENABLED, {}}), system_state.get_module_state("gooseberry"));

    // None of the other modules are ENABLED
    for (auto [name, module_state] : system_state.get_module_states()) {
        if (name != "fruit-salad" && name != "gooseberry") {
            CPPUNIT_ASSERT_EQUAL(libdnf5::system::ModuleState({"", ModuleStatus::AVAILABLE, {}}), module_state);
        }
    }
}


void ModuleTest::test_module_enable_default() {
    add_repo_repomd("repomd-modules");

    // Add module enable goal operation
    libdnf5::Goal goal(base);
    goal.add_module_enable("NoStaticContext", libdnf5::GoalJobSettings());
    auto transaction = goal.resolve();

    // Active modules contain the enabled NoStaticContext with its default stream latest, its dependency gooseberry
    // and the default stream of module berries
    std::vector<std::string> expected_active_module_specs{
        "NoStaticContext:latest:1::x86_64",
        "berries:main:4:6c81f848:x86_64",
        "gooseberry:5.5:2:72aaf46b6:x86_64",
        "gooseberry:5.5:3:72aaf46b6:x86_64"};
    std::vector<std::string> active_module_specs;
    for (auto & module_item : base.get_module_sack()->get_active_modules()) {
        active_module_specs.push_back(module_item->get_full_identifier());
    }
    std::sort(active_module_specs.begin(), active_module_specs.end());
    CPPUNIT_ASSERT_EQUAL(expected_active_module_specs, active_module_specs);

    // Run the transaction
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::Transaction::TransactionRunResult::SUCCESS, transaction.run());

    auto system_state = (base.*get(priv_impl()))->get_system_state();

    // Module NoStaticContext is ENABLED because it was explicitly enabled
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::system::ModuleState({"latest", ModuleStatus::ENABLED, {}}),
        system_state.get_module_state("NoStaticContext"));
    // Module goosebery is ENABLED because module NoStaticContext requires it
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::system::ModuleState({"5.5", ModuleStatus::ENABLED, {}}), system_state.get_module_state("gooseberry"));

    // None of the other modules are ENABLED
    for (auto [name, module_state] : system_state.get_module_states()) {
        if (name != "NoStaticContext" && name != "gooseberry") {
            CPPUNIT_ASSERT_EQUAL(libdnf5::system::ModuleState({"", ModuleStatus::AVAILABLE, {}}), module_state);
        }
    }
}


void ModuleTest::test_module_disable() {
    add_repo_repomd("repomd-modules");

    // Add module disable goal operation
    libdnf5::Goal goal(base);
    goal.add_module_disable("fruit-salad:main", libdnf5::GoalJobSettings());
    auto transaction = goal.resolve();

    // Active modules contain the the default streams of modules NoStaticContext and berries and their dependency gooseberry:5.5
    std::vector<std::string> expected_active_module_specs{
        "NoStaticContext:latest:1::x86_64",
        "berries:main:4:6c81f848:x86_64",
        "gooseberry:5.5:2:72aaf46b6:x86_64",
        "gooseberry:5.5:3:72aaf46b6:x86_64"};
    std::vector<std::string> active_module_specs;
    for (auto & module_item : base.get_module_sack()->get_active_modules()) {
        active_module_specs.push_back(module_item->get_full_identifier());
    }
    std::sort(active_module_specs.begin(), active_module_specs.end());
    CPPUNIT_ASSERT_EQUAL(expected_active_module_specs, active_module_specs);

    // Run the transaction
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::Transaction::TransactionRunResult::SUCCESS, transaction.run());

    auto system_state = (base.*get(priv_impl()))->get_system_state();

    // Module fruit-salat is DISABLED because it was explicitly disabled
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::system::ModuleState({"", ModuleStatus::DISABLED, {}}), system_state.get_module_state("fruit-salad"));

    // None of the other modules is DISABLED
    for (auto [name, module_state] : system_state.get_module_states()) {
        if (name != "fruit-salad") {
            CPPUNIT_ASSERT_EQUAL(libdnf5::system::ModuleState({"", ModuleStatus::AVAILABLE, {}}), module_state);
        }
    }
}


void ModuleTest::test_module_disable_enabled() {
    add_repo_repomd("repomd-modules");

    // Set state of modules NoStaticContext and berries to ENABLED
    auto & system_state = (base.*get(priv_impl()))->get_system_state();
    system_state.set_module_state(
        "NoStaticContext", libdnf5::system::ModuleState({"latest", ModuleStatus::ENABLED, {}}));
    system_state.set_module_state("berries", libdnf5::system::ModuleState({"main", ModuleStatus::ENABLED, {}}));

    // Add module disable goal operation
    libdnf5::Goal goal(base);
    goal.add_module_disable("NoStaticContext", libdnf5::GoalJobSettings());
    goal.add_module_disable("berries", libdnf5::GoalJobSettings());
    auto transaction = goal.resolve();

    // Active modules don't contain anything, because the only modules that had default streams were disabled
    std::vector<std::string> active_module_specs;
    for (auto & module_item : base.get_module_sack()->get_active_modules()) {
        active_module_specs.push_back(module_item->get_full_identifier());
    }
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>{}, active_module_specs);

    // Run the transaction
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::Transaction::TransactionRunResult::SUCCESS, transaction.run());

    // Modules NoStaticContext and berries are DISABLED because they were explicitly disabled
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::system::ModuleState({"", ModuleStatus::DISABLED, {}}),
        system_state.get_module_state("NoStaticContext"));
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::system::ModuleState({"", ModuleStatus::DISABLED, {}}), system_state.get_module_state("berries"));

    // None of the other modules is DISABLED
    for (auto [name, module_state] : system_state.get_module_states()) {
        if (name != "NoStaticContext" && name != "berries") {
            CPPUNIT_ASSERT_EQUAL(libdnf5::system::ModuleState({"", ModuleStatus::AVAILABLE, {}}), module_state);
        }
    }
}


void ModuleTest::test_module_reset() {
    // Set state of module berries to ENABLED
    // This has to be done before the repos are loaded and modules initialized
    // in load_repos in add_repo_repomd.
    (base.*get(priv_impl()))
        ->get_system_state()
        .set_module_state("berries", libdnf5::system::ModuleState({"main", ModuleStatus::ENABLED, {}}));

    add_repo_repomd("repomd-modules");

    // Add module reset goal operation
    libdnf5::Goal goal(base);
    goal.add_module_reset("berries", libdnf5::GoalJobSettings());
    auto transaction = goal.resolve();

    // Active modules contain the the default streams of modules NoStaticContext and berries and their dependency
    // gooseberry:5.5
    std::vector<std::string> expected_active_module_specs{
        "NoStaticContext:latest:1::x86_64",
        "berries:main:4:6c81f848:x86_64",
        "gooseberry:5.5:2:72aaf46b6:x86_64",
        "gooseberry:5.5:3:72aaf46b6:x86_64"};
    std::vector<std::string> active_module_specs;
    for (auto & module_item : base.get_module_sack()->get_active_modules()) {
        active_module_specs.push_back(module_item->get_full_identifier());
    }
    std::sort(active_module_specs.begin(), active_module_specs.end());
    CPPUNIT_ASSERT_EQUAL(expected_active_module_specs, active_module_specs);

    // Run the transaction
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::Transaction::TransactionRunResult::SUCCESS, transaction.run());

    auto system_state = (base.*get(priv_impl()))->get_system_state();

    // None of the modules is DISABLED
    for (auto [name, module_state] : system_state.get_module_states()) {
        CPPUNIT_ASSERT_EQUAL(libdnf5::system::ModuleState({"", ModuleStatus::AVAILABLE, {}}), module_state);
    }
}


void ModuleTest::test_module_globs() {
    add_repo_repomd("repomd-modules");

    // Add module enable goal operation with module_spec containing globs
    libdnf5::Goal goal(base);
    goal.add_module_enable("*salad", libdnf5::GoalJobSettings());
    auto transaction = goal.resolve();

    // Active modules contain the enabled fruit-salad and vegetable-salad, its dependency gooseberry and the default
    // streams of modules NoStaticContext and berries
    std::vector<std::string> expected_active_module_specs{
        "NoStaticContext:latest:1::x86_64",
        "berries:main:4:6c81f848:x86_64",
        "fruit-salad:main:12:2241675a:x86_64",
        "gooseberry:5.5:2:72aaf46b6:x86_64",
        "gooseberry:5.5:3:72aaf46b6:x86_64",
        "vegetable-salad:latest:1:aaa456b:x86_64"};
    std::vector<std::string> active_module_specs;
    for (auto & module_item : base.get_module_sack()->get_active_modules()) {
        active_module_specs.push_back(module_item->get_full_identifier());
    }
    std::sort(active_module_specs.begin(), active_module_specs.end());
    CPPUNIT_ASSERT_EQUAL(expected_active_module_specs, active_module_specs);
}

#endif  // WITH_MODULEMD
