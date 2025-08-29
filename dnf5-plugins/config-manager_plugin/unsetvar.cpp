// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "unsetvar.hpp"

#include "shared.hpp"

#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <filesystem>

namespace dnf5 {

using namespace libdnf5;

void ConfigManagerUnsetVarCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Unset/remove variables");

    auto vars =
        parser.add_new_positional_arg("variables", cli::ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    vars->set_description("List of variables to unset");
    vars->set_parse_hook_func(
        [this]([[maybe_unused]] cli::ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                std::string var_name{argv[i]};

                check_variable_name(var_name);

                // Save the variable for later removing.
                vars_to_remove.insert(var_name);
            }
            return true;
        });
    cmd.register_positional_arg(vars);
}


void ConfigManagerUnsetVarCommand::configure() {
    auto & ctx = get_context();
    if (!vars_to_remove.empty()) {
        const auto & vars_dir = get_last_vars_dir_path(ctx.get_base().get_config());
        if (vars_dir.empty()) {
            throw ConfigManagerError(M_("Missing path to vars directory"));
        }

        if (!std::filesystem::exists(vars_dir)) {
            write_warning(
                *ctx.get_base().get_logger(),
                M_("config-manager: Request to remove variable but vars directory was not found: {}"),
                vars_dir.string());
            return;
        }

        for (const auto & name : vars_to_remove) {
            const auto filepath = vars_dir / name;
            try {
                if (std::filesystem::exists(filepath)) {
                    std::filesystem::remove(filepath);
                } else {
                    write_warning(
                        *ctx.get_base().get_logger(),
                        M_("config-manager: Request to remove variable but it is not present in the vars directory: "
                           "{}"),
                        name);
                }
            } catch (const std::filesystem::filesystem_error & e) {
                throw ConfigManagerError(
                    M_("Cannot remove variable file \"{}\": {}"), filepath.native(), std::string{e.what()});
            }
        }
    }
}

}  // namespace dnf5
