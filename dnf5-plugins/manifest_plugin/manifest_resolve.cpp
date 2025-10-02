// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "manifest.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libpkgmanifest/input/parser.hpp>
#include <libpkgmanifest/manifest/manifest.hpp>
#include <libpkgmanifest/manifest/serializer.hpp>
#include <utils/string.hpp>

#include <regex>

using namespace libdnf5::cli;

namespace dnf5 {

void ManifestResolveCommand::set_argument_parser() {
    ManifestSubcommand::set_argument_parser();

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description(_("Generate a new manifest file using the provided input file"));

    input_path_option = dynamic_cast<libdnf5::OptionPath *>(
        parser.add_init_value(std::make_unique<libdnf5::OptionPath>(DEFAULT_INPUT_FILENAME)));
    auto * input_arg = parser.add_new_named_arg("input");
    input_arg->set_long_name("input");
    input_arg->set_description(_("Input file path"));
    input_arg->set_has_value(true);
    input_arg->link_value(input_path_option);
    cmd.register_named_arg(input_arg);

    use_system_option =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(false)));
    auto * use_system_arg = parser.add_new_named_arg("use-system");
    use_system_arg->set_long_name("use-system");
    use_system_arg->set_description(_("Use installed packages for resolving dependencies"));
    use_system_arg->set_const_value("true");
    use_system_arg->link_value(use_system_option);
    cmd.register_named_arg(use_system_arg);

    per_arch_option =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(false)));
    auto * per_arch_arg = parser.add_new_named_arg("per-arch");
    per_arch_arg->set_long_name("per-arch");
    per_arch_arg->set_description(_("Separate packages by basearch into individual manifest files"));
    per_arch_arg->set_const_value("true");
    per_arch_arg->link_value(per_arch_option);
    cmd.register_named_arg(per_arch_arg);

    srpm_option =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(false)));
    auto * srpm_arg = parser.add_new_named_arg("srpm");
    srpm_arg->set_long_name("srpm");
    srpm_arg->set_description(_("Include source packages in consideration"));
    srpm_arg->set_const_value("true");
    srpm_arg->link_value(srpm_option);
    cmd.register_named_arg(srpm_arg);
}

void ManifestResolveCommand::configure() {
    auto & ctx = get_context();

    input = libpkgmanifest::input::Parser().parse_prototype(input_path_option->get_value());
    arches = input->get_archs();

    ctx.set_load_available_repos(Context::LoadAvailableRepos::NONE);
}

void ManifestResolveCommand::populate_manifest(
    libpkgmanifest::manifest::Manifest & manifest, const std::string & arch, const bool multiarch) {
    auto private_base = create_base_for_arch(arch);
    auto base = private_base->get_weak_ptr();

    // Load repositories
    auto repo_sack = base->get_repo_sack();
    create_repos(*base, input->get_repositories());
    if (srpm_option->get_value()) {
        repo_sack->enable_source_repos();
    }
    set_repo_callbacks(*base);

    get_context().print_info(libdnf5::utils::sformat(_("Updating and loading repositories for arch {}:"), arch));
    if (use_system_option->get_value()) {
        repo_sack->load_repos();
    } else {
        repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);
    }
    get_context().print_info(_("Repositories loaded."));

    // Resolve packages
    libdnf5::Goal goal{base};
    goal.set_allow_erasing(input->get_options().get_allow_erasing());
    for (const auto & spec : input->get_packages().get_installs()) {
        goal.add_rpm_install(spec);
    }
    for (const auto & spec : input->get_packages().get_reinstalls()) {
        goal.add_rpm_reinstall(spec);
    }

    // Populate manifest
    const auto & resolved_pkgs = resolve_goal(goal, *base, srpm_option->get_value());
    add_pkgs_to_manifest(manifest, *base, resolved_pkgs, arch, multiarch);
}

void ManifestResolveCommand::run() {
    libpkgmanifest::manifest::Serializer serializer;
    if (per_arch_option->get_value()) {
        for (const auto & arch : arches) {
            libpkgmanifest::manifest::Manifest manifest;
            populate_manifest(manifest, arch, false);

            std::string path{manifest_path_option->get_value()};
            path = std::regex_replace(path, std::regex("\\.yaml$"), std::format(".{}.yaml", arch));
            serializer.serialize(manifest, path);
        }
    } else {
        libpkgmanifest::manifest::Manifest manifest;
        for (const auto & arch : arches) {
            populate_manifest(manifest, arch, arches.size() > 1);
        }
        serializer.serialize(manifest, manifest_path_option->get_value());
    }
}

}  // namespace dnf5
