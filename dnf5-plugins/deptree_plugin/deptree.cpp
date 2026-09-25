// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "deptree.hpp"

#include "deptree_graph.hpp"

#include <dnf5/shared_options.hpp>
#include <json-c/json.h>
#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5/conf/option_string_list.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/format.hpp>

#include <iostream>
#include <set>

namespace dnf5 {

void DeptreeCommand::set_parent_command() {
    auto * parent = get_session().get_argument_parser().get_root_command();
    parent->register_command(get_argument_parser_command());
    parent->get_group("software_management_commands").register_argument(get_argument_parser_command());
}

void DeptreeCommand::set_argument_parser() {
    auto & parser = get_context().get_argument_parser();
    auto & command = *get_argument_parser_command();
    command.set_description(_("Display package dependency trees."));

    auto specs = parser.add_new_positional_arg(
        "package-spec", libdnf5::cli::ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    specs->set_description(_("Package names, NEVRAs, or glob patterns."));
    specs->set_parse_hook_func([this](auto *, int argc, const char * const argv[]) {
        for (int index = 0; index < argc; ++index) {
            pkg_specs.emplace_back(argv[index]);
        }
        return true;
    });
    command.register_positional_arg(specs);

    reverse = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "reverse", '\0', _("Show packages requiring each package."), false);
    flat = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "flat", '\0', _("Print a sorted, deduplicated package list."), false);
    show_requires = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "show-requires", '\0', _("Show requirement capability nodes."), false);
    show_duplicates = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "showduplicates", '\0', _("Include all matching package versions."), false);
    installed = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "installed", '\0', _("Display dependency trees for installed packages only."), false);

    auto * arch_arg = parser.add_new_named_arg("arch");
    arch_arg->set_long_name("arch");
    arch_arg->set_description(_("Limit to packages of the specified architectures."));
    arch_arg->set_has_value(true);
    arch_arg->set_arg_value_help("ARCH[,ARCH...]");
    arch_arg->set_parse_hook_func([this](auto *, const char *, const char * value) {
        const libdnf5::OptionStringList values(value);
        arches.insert(values.get_value().begin(), values.get_value().end());
        return true;
    });
    command.register_named_arg(arch_arg);

    depth_option = dynamic_cast<libdnf5::OptionNumber<std::int32_t> *>(
        parser.add_init_value(std::make_unique<libdnf5::OptionNumber<std::int32_t>>(-1)));
    auto * depth_arg = parser.add_new_named_arg("depth");
    depth_arg->set_long_name("depth");
    depth_arg->set_description(_("Limit traversal to N dependency levels."));
    depth_arg->set_has_value(true);
    depth_arg->set_arg_value_help("N");
    depth_arg->link_value(depth_option);
    command.register_named_arg(depth_arg);

    dependency_types_option =
        dynamic_cast<libdnf5::OptionStringSet *>(parser.add_init_value(std::make_unique<libdnf5::OptionStringSet>(
            libdnf5::OptionStringSet::ValueType{"requires"},
            "requires|recommends|suggests|supplements|enhances",
            false)));
    auto * types_arg = parser.add_new_named_arg("types");
    types_arg->set_long_name("types");
    types_arg->set_description(
        _("Comma-separated dependency types: requires, recommends, suggests, supplements, enhances."));
    types_arg->set_has_value(true);
    types_arg->set_arg_value_help("LIST");
    types_arg->link_value(dependency_types_option);
    command.register_named_arg(types_arg);

    create_json_option(*this);
}

void DeptreeCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(installed->get_value());
    context.set_load_available_repos(
        installed->get_value() ? Context::LoadAvailableRepos::NONE : Context::LoadAvailableRepos::ENABLED);
}

void DeptreeCommand::run() {
    auto & base = get_context().get_base();
    libdnf5::rpm::PackageQuery available(base, libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES);
    if (!arches.empty()) {
        available.filter_arch(std::vector<std::string>(arches.begin(), arches.end()));
    }
    libdnf5::ResolveSpecSettings settings;
    settings.set_with_nevra(true);
    settings.set_with_provides(false);
    settings.set_with_filenames(false);
    settings.set_with_binaries(false);

    libdnf5::rpm::PackageQuery root_query(base, libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);
    for (const auto & spec : pkg_specs) {
        libdnf5::rpm::PackageQuery matches(available);
        const auto resolved_spec = matches.resolve_pkg_spec(spec, settings, true);
        if (!resolved_spec.first) {
            std::cerr << libdnf5::utils::sformat(_("No match for argument \"{}\"."), spec) << std::endl;
            continue;
        }
        if (!show_duplicates->get_value() && resolved_spec.second.get_arch().empty()) {
            matches.filter_latest_evr();
        }
        root_query |= matches;
    }

    GraphBuilder builder(
        base,
        dependency_types_option->get_value(),
        arches,
        reverse->get_value(),
        show_requires->get_value(),
        show_duplicates->get_value(),
        depth_option->get_value());
    const auto graph = builder.build(root_query);

    if (flat->get_value()) {
        std::set<std::string> packages;
        for (const auto & [id, node] : graph.nodes) {
            if (node.type == Node::Type::PACKAGE) {
                packages.insert(node.label);
            }
        }
        if (get_context().get_json_output_requested()) {
            json_object * result = json_object_new_array();
            for (const auto & package : packages) {
                json_object_array_add(result, json_object_new_string(package.c_str()));
            }
            std::cout << json_object_to_json_string_ext(result, JSON_C_TO_STRING_PRETTY) << std::endl;
            json_object_put(result);
        } else {
            for (const auto & package : packages) {
                std::cout << package << std::endl;
            }
        }
        return;
    }

    if (get_context().get_json_output_requested()) {
        print_json(graph);
    } else {
        print_tree(graph);
    }
}

}  // namespace dnf5
