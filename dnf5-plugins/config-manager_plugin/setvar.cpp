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

#include "setvar.hpp"

#include "shared.hpp"

#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <fstream>

namespace dnf5 {

using namespace libdnf5;

void ConfigManagerSetVarCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Set variables");

    auto vars_vals =
        parser.add_new_positional_arg("varvals", cli::ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    vars_vals->set_description("List of variables with values. Format: \"variable=value\"");
    vars_vals->set_parse_hook_func(
        [this, &ctx]([[maybe_unused]] cli::ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                auto value = argv[i];
                auto val = strchr(value + 1, '=');
                if (!val) {
                    throw cli::ArgumentParserError(
                        M_("{}: Badly formatted argument value \"{}\""), std::string{"varval"}, std::string{value});
                }
                std::string var_name{value, val};
                std::string var_value{val + 1};

                check_variable_name(var_name);

                // Test that the variable is not read-only.
                auto vars = ctx.get_base().get_vars();
                if (vars->is_read_only(var_name)) {
                    throw ConfigManagerError(
                        M_("Cannot set \"{}\": Variable \"{}\" is read-only"), std::string{value}, var_name);
                }

                // Save the variable for later writing to a file.
                const auto [it, inserted] = setvars.insert({var_name, var_value});
                if (!inserted) {
                    if (it->second != var_value) {
                        throw ConfigManagerError(
                            M_("Sets the \"{}\" variable again with a different value: \"{}\" != \"{}\""),
                            var_name,
                            it->second,
                            var_value);
                    }
                }
            }
            return true;
        });
    cmd.register_positional_arg(vars_vals);

    auto create_missing_dirs_opt = parser.add_new_named_arg("create-missing-dir");
    create_missing_dirs_opt->set_long_name("create-missing-dir");
    create_missing_dirs_opt->set_description("Allow to create missing directories");
    create_missing_dirs_opt->set_has_value(false);
    create_missing_dirs_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        create_missing_dirs = true;
        return true;
    });
    cmd.register_named_arg(create_missing_dirs_opt);
}


void ConfigManagerSetVarCommand::configure() {
    auto & ctx = get_context();

    if (!setvars.empty()) {
        const auto & vars_dir = get_last_vars_dir_path(ctx.get_base().get_config());
        if (vars_dir.empty()) {
            throw ConfigManagerError(M_("Missing path to vars directory"));
        }
        resolve_missing_dir(vars_dir, create_missing_dirs);

        for (const auto & [name, value] : setvars) {
            const auto filepath = vars_dir / name;
            std::ofstream file;
            file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
            try {
                file.open(filepath, std::ios_base::trunc | std::ios_base::binary);
                file << value;
            } catch (const std::ios_base::failure & e) {
                throw ConfigManagerError(
                    M_("Cannot write variable to file \"{}\": {}"), filepath.native(), std::string{e.what()});
            }
            set_file_permissions(filepath);
        }
    }
}

}  // namespace dnf5
