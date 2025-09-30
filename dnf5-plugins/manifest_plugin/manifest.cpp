// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "manifest.hpp"

#include "download_callbacks.hpp"

#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/logger/logger.hpp>
#include <libdnf5/repo/repo_sack.hpp>
#include <libdnf5/rpm/rpm_signature.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libpkgmanifest/manifest/parser.hpp>

#include <regex>

using namespace libdnf5::cli;

libdnf5::rpm::Nevra nevra_manifest_to_dnf(const libpkgmanifest::manifest::Nevra & manifest_nevra) {
    libdnf5::rpm::Nevra dnf_nevra;
    dnf_nevra.set_name(manifest_nevra.get_name());
    dnf_nevra.set_epoch(manifest_nevra.get_epoch());
    dnf_nevra.set_version(manifest_nevra.get_version());
    dnf_nevra.set_release(manifest_nevra.get_release());
    dnf_nevra.set_arch(manifest_nevra.get_arch());
    return dnf_nevra;
}

std::filesystem::path get_manifest_path(libdnf5::OptionPath & option, const std::string & arch) {
    std::string path{option.get_value()};

    if (option.get_priority() <= libdnf5::Option::Priority::DEFAULT) {
        const auto & arch_path = std::regex_replace(path, std::regex("\\.yaml$"), std::format(".{}.yaml", arch));
        const bool path_exists = std::filesystem::exists(path);
        const bool arch_path_exists = std::filesystem::exists(arch_path);
        if (path_exists && arch_path_exists) {
            throw libdnf5::cli::CommandExitError(
                1,
                M_("Multiple manifest files detected in the directory. Either use filename with the '.{}.yaml' suffix, "
                   "keep only one file, or specify a file explicitly using the '--manifest' option."),
                arch);
        }
        if (!path_exists) {
            if (arch_path_exists) {
                path = arch_path;
            } else {
                throw libdnf5::cli::CommandExitError(1, M_("Manifest file '{}' does not exist"), path);
            }
        }
    }
    return path;
}

class KeyImportRepoCB : public libdnf5::repo::RepoCallbacks2_1 {
public:
    explicit KeyImportRepoCB(libdnf5::ConfigMain & config) : config(&config) {}

    bool repokey_import(const libdnf5::rpm::KeyInfo & key_info) override {
        if (config->get_assumeno_option().get_value()) {
            return false;
        }

        std::cerr << libdnf5::utils::sformat(_("Importing OpenPGP key 0x{}:\n"), key_info.get_short_key_id());
        for (auto & user_id : key_info.get_user_ids()) {
            std::cerr << libdnf5::utils::sformat(_(" UserID     : \"{}\"\n"), user_id);
        }
        std::cerr << libdnf5::utils::sformat(
            _(" Fingerprint: {}\n"
              " From       : {}\n"),
            key_info.get_fingerprint(),
            key_info.get_url());

        return libdnf5::cli::utils::userconfirm::userconfirm(*config);
    }

    void repokey_imported([[maybe_unused]] const libdnf5::rpm::KeyInfo & key_info) override {
        std::cerr << _("The key was successfully imported.") << std::endl;
    }

    bool repokey_remove(const libdnf5::rpm::KeyInfo & key_info, const libdnf5::Message & removal_info) override {
        if (config->get_assumeno_option().get_value()) {
            return false;
        }

        std::cerr << libdnf5::utils::sformat(
                         _("The following OpenPGP key (0x{}) is about to be removed:"), key_info.get_short_key_id())
                  << std::endl;
        std::cerr << libdnf5::utils::sformat(_(" Reason     : {}\n"), removal_info.format(true));
        for (auto & user_id : key_info.get_user_ids()) {
            std::cerr << libdnf5::utils::sformat(_(" UserID     : \"{}\"\n"), user_id);
        }
        std::cerr << libdnf5::utils::sformat(_(" Fingerprint: {}\n"), key_info.get_fingerprint());

        std::cerr << std::endl << _("As a result, installing packages signed with this key will fail.") << std::endl;
        std::cerr << _("It is recommended to remove the expired key to allow importing") << std::endl;
        std::cerr << _("an updated key. This might leave already installed packages unverifiable.") << std::endl
                  << std::endl;

        std::cerr << _("The system will now proceed with removing the key.") << std::endl;

        return libdnf5::cli::utils::userconfirm::userconfirm(*config);
    }

    void repokey_removed([[maybe_unused]] const libdnf5::rpm::KeyInfo & key_info) override {
        std::cerr << libdnf5::utils::sformat(_("Key 0x{} was successfully removed."), key_info.get_short_key_id())
                  << std::endl;
    }

private:
    libdnf5::ConfigMain * config;
};

void set_repo_callbacks(libdnf5::Base & base) {
    libdnf5::repo::RepoQuery repos{base};
    for (auto & repo : repos) {
        repo->set_callbacks(std::make_unique<KeyImportRepoCB>(base.get_config()));
    }
}

namespace dnf5 {

void ManifestCommand::pre_configure() {
    throw_missing_command();
}

void ManifestCommand::set_parent_command() {
    auto * parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * this_cmd = get_argument_parser_command();
    parent_cmd->register_command(this_cmd);

    auto & group = parent_cmd->get_group("software_management_commands");
    group.register_argument(this_cmd);
}

void ManifestCommand::register_subcommands() {
    register_subcommand(std::make_unique<ManifestNewCommand>(get_context()));
    register_subcommand(std::make_unique<ManifestDownloadCommand>(get_context()));
    register_subcommand(std::make_unique<ManifestInstallCommand>(get_context()));
}

void ManifestCommand::set_argument_parser() {
    get_argument_parser_command()->set_description(_("Operations for working with RPM package manifest files"));
}

void ManifestSubcommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    manifest_path_option = dynamic_cast<libdnf5::OptionPath *>(
        parser.add_init_value(std::make_unique<libdnf5::OptionPath>(DEFAULT_MANIFEST_FILENAME)));
    auto * manifest_arg = parser.add_new_named_arg("manifest");
    manifest_arg->set_long_name("manifest");
    manifest_arg->set_description(_("Manifest file path"));
    manifest_arg->set_has_value(true);
    manifest_arg->link_value(manifest_path_option);
    cmd.register_named_arg(manifest_arg);
}

void ManifestSubcommand::pre_configure() {
    auto & ctx = get_context();
    auto & base = ctx.get_base();

    config_before_setup = std::make_unique<libdnf5::ConfigMain>();
    config_before_setup->load_from_config(base.get_config());
}

std::unique_ptr<libdnf5::Base> ManifestSubcommand::create_base_for_arch(const std::string & arch) const {
    auto base = std::make_unique<libdnf5::Base>();

    base->get_config().load_from_config(*config_before_setup);

    base->get_config().get_skip_broken_option().set(false);
    base->get_config().get_skip_unavailable_option().set(false);

    base->get_vars()->set("arch", arch, libdnf5::Vars::Priority::RUNTIME);

    auto download_callbacks_uptr = std::make_unique<dnf5::DownloadCallbacks>();
    auto * download_callbacks = download_callbacks_uptr.get();
    download_callbacks->set_show_total_bar_limit(static_cast<std::size_t>(-1));

    base->set_download_callbacks(std::move(download_callbacks_uptr));

    base->load_config();
    base->setup();

    return base;
}

void ManifestSubcommand::create_repos(libdnf5::Base & base, libpkgmanifest::common::Repositories repos) const {
    auto repo_sack = base.get_repo_sack();
    for (const auto & manifest_repo : repos) {
        auto repo = repo_sack->create_repo(manifest_repo.get_id());

        repo->set_callbacks(std::make_unique<KeyImportRepoCB>(base.get_config()));

        auto & repo_config = repo->get_config();
        // For now, disable `pkg_gpgcheck` on all repositories until libpkgmanifest is capable of tracking which repositories should have `pkg_gpgcheck=true`.
        repo_config.get_pkg_gpgcheck_option().set(false);
        const auto & vars = base.get_vars();
        if (!manifest_repo.get_metalink().empty()) {
            const auto & metalink = vars->substitute(manifest_repo.get_metalink());
            repo_config.get_metalink_option().set(metalink);
        } else if (!manifest_repo.get_mirrorlist().empty()) {
            const auto & mirrorlist = vars->substitute(manifest_repo.get_mirrorlist());
            repo_config.get_mirrorlist_option().set(mirrorlist);
        } else if (!manifest_repo.get_baseurl().empty()) {
            const auto & baseurl = vars->substitute(manifest_repo.get_baseurl());
            repo_config.get_baseurl_option().set(baseurl);
        } else {
            throw libdnf5::RuntimeError(M_("Repository \"{}\" has no baseurl"), manifest_repo.get_id());
        }
    }
}

}  // namespace dnf5
