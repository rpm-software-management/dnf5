// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "manifest.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libpkgmanifest/input/parser.hpp>
#include <libpkgmanifest/manifest/manifest.hpp>
#include <libpkgmanifest/manifest/parser.hpp>
#include <libpkgmanifest/manifest/serializer.hpp>
#include <utils/string.hpp>

using namespace libdnf5::cli;

namespace dnf5 {

void ManifestInstallCommand::set_argument_parser() {
    ManifestSubcommand::set_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Install all packages specified in the manifest file"));
}

void ManifestInstallCommand::pre_configure() {
    ManifestSubcommand::pre_configure();

    auto & ctx = get_context();
    ctx.set_create_repos(false);
    ctx.set_load_system_repo(true);
    ctx.set_load_available_repos(Context::LoadAvailableRepos::ALL);
}

void ManifestInstallCommand::configure() {
    auto & ctx = get_context();
    auto & base = ctx.get_base();

    const auto & arch = base.get_vars()->get_value("basearch");

    const auto & manifest_path = get_manifest_path(*manifest_path_option, arch);

    std::filesystem::path destdir{manifest_path.stem()};
    base.get_config().get_destdir_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, destdir);

    base.get_config().get_skip_broken_option().set(false);
    base.get_config().get_skip_unavailable_option().set(false);

    manifest = libpkgmanifest::manifest::Parser().parse(manifest_path);
    create_repos(base, manifest.get_repositories());
}

void ManifestInstallCommand::run() {
    auto & ctx = get_context();
    auto & base = ctx.get_base();

    auto * goal = ctx.get_goal();
    const auto & arch = base.get_vars()->get_value("basearch");
    for (auto & manifest_pkg : manifest.get_packages().get(arch)) {
        libdnf5::GoalJobSettings settings;
        settings.set_to_repo_ids({manifest_pkg.get_repo_id()});
        goal->add_install(manifest_pkg.get_nevra().to_string(), settings);
    }
}

}  // namespace dnf5
