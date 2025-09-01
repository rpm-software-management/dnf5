// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "shared_options.hpp"

#include "utils/url.hpp"

#include <libdnf5-cli/argument_parser.hpp>

#include <filesystem>

namespace {

static void normalize_paths(int specs_count, const char * const specs[], std::vector<std::string> & pkg_specs) {
    const std::string_view ext(".rpm");
    std::set<std::string_view> unique_items;
    for (int i = 0; i < specs_count; ++i) {
        const std::string_view spec(specs[i]);
        if (auto [it, inserted] = unique_items.emplace(spec); inserted) {
            if (!libdnf5::utils::url::is_url(specs[i]) && spec.length() > ext.length() && spec.ends_with(ext)) {
                // convert local file paths to absolute path
                pkg_specs.emplace_back(std::filesystem::canonical(spec));
            } else {
                pkg_specs.emplace_back(spec);
            }
        }
    }
}

}  // anonymous namespace

namespace dnfdaemon::client {

libdnf5::cli::ArgumentParser::PositionalArg * pkg_specs_argument(
    libdnf5::cli::ArgumentParser & parser, int nargs, std::vector<std::string> & pkg_specs) {
    auto specs = parser.add_new_positional_arg("pkg_specs", nargs, nullptr, nullptr);
    specs->set_parse_hook_func(
        [&]([[maybe_unused]] libdnf5::cli::ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            normalize_paths(argc, argv, pkg_specs);
            return true;
        });
    return specs;
}

libdnf5::cli::ArgumentParser::NamedArg * create_offline_option(
    libdnf5::cli::ArgumentParser & parser, libdnf5::Option * value) {
    auto offline = parser.add_new_named_arg("offline");
    offline->set_long_name("offline");
    offline->set_description("Store the transaction to be performed offline");
    offline->set_const_value("true");
    offline->link_value(value);
    return offline;
}

}  // namespace dnfdaemon::client
