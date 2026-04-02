// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "manifest.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libpkgmanifest/input/parser.hpp>
#include <libpkgmanifest/manifest/manifest.hpp>
#include <libpkgmanifest/manifest/parser.hpp>
#include <libpkgmanifest/manifest/serializer.hpp>
#include <utils/string.hpp>

#include <ranges>

using namespace libdnf5::cli;

namespace dnf5 {

void ManifestDownloadCommand::set_argument_parser() {
    ManifestSubcommand::set_argument_parser();

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description(_("Download all packages specified in the manifest file to disk."));

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
    auto * srpm_arg = parser.add_new_named_arg("source");
    srpm_arg->set_long_name("source");
    srpm_arg->set_description(_("Download source packages"));
    srpm_arg->set_const_value("true");
    srpm_arg->link_value(srpm_option);
    cmd.register_named_arg(srpm_arg);

    create_destdir_option(*this);
}

void ManifestDownloadCommand::pre_configure() {
    ManifestSubcommand::pre_configure();

    auto & ctx = get_context();
    ctx.set_create_repos(false);
    ctx.set_load_system_repo(false);
    ctx.set_load_available_repos(Context::LoadAvailableRepos::NONE);
}

void ManifestDownloadCommand::configure() {
    auto & ctx = get_context();

    if (arch_option->get_priority() > libdnf5::Option::Priority::DEFAULT) {
        arches = std::vector<std::string>(arch_option->get_value().begin(), arch_option->get_value().end());
    } else {
        arches = {ctx.get_base().get_vars()->get_value("basearch")};
    }

    for (const auto & arch : arches) {
        manifest_paths[arch] = get_manifest_path(*manifest_path_option, arch);
    }
}

void ManifestDownloadCommand::download_packages(
    libpkgmanifest::manifest::Manifest & manifest,
    const std::string & arch,
    const std::filesystem::path & default_destdir) {
    auto & ctx = get_context();

    auto base = create_base_for_arch(arch);
    const auto & manifest_path_base = std::filesystem::path{manifest_path_option->get_value()}.stem();
    base->get_config().get_destdir_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, default_destdir);

    // Load repositories
    auto repo_sack = base->get_repo_sack();
    create_repos(*base, manifest.get_repositories());
    if (srpm_option->get_value()) {
        repo_sack->enable_source_repos();
    }
    set_repo_callbacks(*base);
    ctx.print_info(libdnf5::utils::sformat(_("Updating and loading repositories for arch {}:"), arch));
    repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);
    ctx.print_info(_("Repositories loaded."));

    // Download packages
    libdnf5::repo::PackageDownloader downloader{*base};
    downloader.force_keep_packages(true);

    for (auto & manifest_pkg : manifest.get_packages().get(arch, srpm_option->get_value())) {
        libdnf5::rpm::PackageQuery query{*base};
        query.filter_repo_id(manifest_pkg.get_repo_id());
        const auto & nevra = nevra_manifest_to_dnf(manifest_pkg.get_nevra());
        query.filter_nevra(nevra);
        if (query.empty()) {
            throw libdnf5::cli::CommandExitError(1, M_("No package {} available."), to_nevra_string(nevra));
        }
        const auto & pkg = *query.begin();
        downloader.add(*query.begin());
    }
    downloader.download();
}

void ManifestDownloadCommand::run() {
    std::optional<std::filesystem::path> last_manifest_path;
    std::optional<libpkgmanifest::manifest::Manifest> manifest;
    for (const auto & arch : arches) {
        const auto & manifest_path = manifest_paths[arch];
        if (!manifest.has_value() || last_manifest_path != manifest_path) {
            manifest = libpkgmanifest::manifest::Parser().parse(manifest_path);
        }
        last_manifest_path = manifest_path;
        const auto & manifest_path_base = manifest_path.stem();
        const auto & default_destdir = std::filesystem::absolute(manifest_path_base);
        download_packages(*manifest, arch, default_destdir);
    }
}

}  // namespace dnf5
