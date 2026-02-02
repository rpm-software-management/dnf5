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

void ManifestNewCommand::set_argument_parser() {
    ManifestSubcommand::set_argument_parser();

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description(
        _("Generate a new manifest file using the provided package specs, input file, or system state"));

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

    arch_option = dynamic_cast<libdnf5::OptionStringList *>(
        parser.add_init_value(std::make_unique<libdnf5::OptionStringList>(std::vector<std::string>())));
    auto * arch_arg = parser.add_new_named_arg("arch");
    arch_arg->set_long_name("arch");
    arch_arg->set_description("Explicitly specify basearches to use");
    arch_arg->set_has_value(true);
    arch_arg->set_arg_value_help("<ARCH>,...");
    arch_arg->link_value(arch_option);
    cmd.register_named_arg(arch_arg);

    srpm_option =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(false)));
    auto * srpm_arg = parser.add_new_named_arg("srpm");
    srpm_arg->set_long_name("srpm");
    srpm_arg->set_description(_("Include source packages in consideration"));
    srpm_arg->set_const_value("true");
    srpm_arg->link_value(srpm_option);
    cmd.register_named_arg(srpm_arg);

    auto keys = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    keys->set_description("List of [<package-spec-NPFB>|@<group-spec>|@<environment-spec>]");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return ctx.match_specs(arg, true, false, true, false); });
    cmd.register_positional_arg(keys);
}

void ManifestNewCommand::configure() {
    auto & ctx = get_context();

    if (use_system_option->get_value()) {
        use_system_repository = true;
    }
    if (pkg_specs.empty()) {
        use_system_repository = true;
        generate_system_snapshot = true;
    }

    if (arch_option->get_priority() > libdnf5::Option::Priority::DEFAULT) {
        arches = std::vector<std::string>(arch_option->get_value().begin(), arch_option->get_value().end());
    } else {
        arches = {ctx.get_base().get_vars()->get_value("basearch")};
    }

    ctx.set_load_available_repos(Context::LoadAvailableRepos::NONE);
}

void ManifestNewCommand::populate_manifest(
    libpkgmanifest::manifest::Manifest & manifest, const std::string & arch, const bool multiarch) {
    auto & ctx = get_context();
    auto private_base = create_base_for_arch(arch);
    auto base = private_base->get_weak_ptr();

    // Load repositories
    auto repo_sack = base->get_repo_sack();
    load_host_repos(ctx, *base);

    if (srpm_option->get_value()) {
        repo_sack->enable_source_repos();
    }
    set_repo_callbacks(*base);

    get_context().print_info(libdnf5::utils::sformat(_("Updating and loading repositories for arch {}:"), arch));
    if (use_system_repository) {
        repo_sack->load_repos();
    } else {
        repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);
    }
    get_context().print_info(_("Repositories loaded."));

    // Resolve packages and populate manifest
    const auto & resolved_pkgs = resolve_pkgs(*base);
    add_pkgs_to_manifest(manifest, *base, resolved_pkgs, arch, multiarch);
}

void ManifestNewCommand::run() {
    libpkgmanifest::manifest::Serializer serializer;
    if (per_arch_option->get_value()) {
        for (const auto & arch : arches) {
            libpkgmanifest::manifest::Manifest manifest;
            populate_manifest(manifest, arch, false);

            std::string path{manifest_path_option->get_value()};
            path = std::regex_replace(path, std::regex("\\.yaml$"), fmt::format(".{}.yaml", arch));
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

std::vector<libdnf5::rpm::Package> ManifestNewCommand::resolve_pkgs(libdnf5::Base & base) {
    if (generate_system_snapshot) {
        libdnf5::rpm::PackageQuery installed_query{base};
        installed_query.filter_installed();
        std::vector<libdnf5::rpm::Package> installed{installed_query.begin(), installed_query.end()};
        return sort_pkgs(std::move(installed));
    }

    std::set<libdnf5::rpm::Package> resolved_pkgs;
    libdnf5::Goal goal(base);
    for (const auto & spec : pkg_specs) {
        goal.add_rpm_install(spec);
    }

    return resolve_goal(goal, base, srpm_option->get_value());
}

}  // namespace dnf5
