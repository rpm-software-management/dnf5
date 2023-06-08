/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "shared_options.hpp"

#include "utils/url.hpp"

#include <filesystem>

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
