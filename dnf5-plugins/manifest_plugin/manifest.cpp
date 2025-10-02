// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "manifest.hpp"

#include "download_callbacks.hpp"

#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/logger/logger.hpp>
#include <libdnf5/repo/repo_errors.hpp>
#include <libdnf5/repo/repo_sack.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/rpm_signature.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libpkgmanifest/manifest/parser.hpp>

#include <regex>

using namespace libdnf5::cli;

std::vector<libdnf5::rpm::Package> sort_pkgs(std::vector<libdnf5::rpm::Package> input) {
    std::sort(input.begin(), input.end(), [](const auto & a, const auto & b) { return a.get_name() < b.get_name(); });
    return input;
}

libdnf5::rpm::Nevra nevra_manifest_to_dnf(const libpkgmanifest::manifest::Nevra & manifest_nevra) {
    libdnf5::rpm::Nevra dnf_nevra;
    dnf_nevra.set_name(manifest_nevra.get_name());
    dnf_nevra.set_epoch(manifest_nevra.get_epoch());
    dnf_nevra.set_version(manifest_nevra.get_version());
    dnf_nevra.set_release(manifest_nevra.get_release());
    dnf_nevra.set_arch(manifest_nevra.get_arch());
    return dnf_nevra;
}

libpkgmanifest::manifest::ChecksumMethod checksum_method_rpm_to_manifest(libdnf5::rpm::Checksum rpm_checksum) {
    switch (rpm_checksum.get_type()) {
        case libdnf5::rpm::Checksum::Type::MD5:
            return libpkgmanifest::manifest::ChecksumMethod::MD5;
        case libdnf5::rpm::Checksum::Type::SHA1:
            return libpkgmanifest::manifest::ChecksumMethod::SHA1;
        case libdnf5::rpm::Checksum::Type::SHA224:
            return libpkgmanifest::manifest::ChecksumMethod::SHA224;
        case libdnf5::rpm::Checksum::Type::SHA256:
            return libpkgmanifest::manifest::ChecksumMethod::SHA256;
        case libdnf5::rpm::Checksum::Type::SHA384:
            return libpkgmanifest::manifest::ChecksumMethod::SHA384;
        case libdnf5::rpm::Checksum::Type::SHA512:
            return libpkgmanifest::manifest::ChecksumMethod::SHA512;
        default:
            throw libdnf5::RuntimeError(M_("Unknown RPM checksum method: {}"), rpm_checksum.get_type_str());
    }
}

std::string get_pkg_location(libdnf5::Base & base, const libdnf5::rpm::Package & dnf_pkg) {
    std::optional<std::string> system_repo_id;
    if (base.get_repo_sack()->has_system_repo()) {
        system_repo_id = base.get_repo_sack()->get_system_repo()->get_id();
    }

    if (dnf_pkg.get_repo_id() != system_repo_id) {
        return dnf_pkg.get_location();
    }
    libdnf5::rpm::PackageQuery query{base};
    query.filter_repo_id(dnf_pkg.get_from_repo_id());
    query.filter_nevra(dnf_pkg.get_nevra());
    if (query.empty()) {
        throw libdnf5::RuntimeError(M_("Couldn't find package {} in available repositories"), dnf_pkg.get_nevra());
    }

    const auto & remote_pkg = *query.begin();
    return remote_pkg.get_location();
}

std::string replace_all(std::string str, const std::string & from, const std::string & to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

std::string get_arch_generic_url(const std::string & url, const std::string & arch) {
    // TODO this is a hack from the original DNF4 manifest plugin. We should instead somehow grab the unsubstituted URL during loading of the repositories from configuration (RepoSack::create_repos_from_file).
    return replace_all(url, arch, "$arch");
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
    register_subcommand(std::make_unique<ManifestResolveCommand>(get_context()));
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
        libdnf5::repo::RepoWeakPtr repo;
        try {
            repo = repo_sack->create_repo(manifest_repo.get_id());
        } catch (const libdnf5::repo::RepoIdAlreadyExistsError & e) {
            throw libdnf5::RuntimeError(
                M_("The stored manifest repository \"{}\" conflicts with an existing repository of the same ID (likely "
                   "added by a libdnf5 plugin)."),
                manifest_repo.get_id());
        }

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

std::vector<libdnf5::rpm::Package> ManifestSubcommand::resolve_goal(
    libdnf5::Goal & goal, libdnf5::Base & base, const bool include_srpms) {
    auto & ctx = get_context();

    // Resolve the goal
    auto transaction = goal.resolve();
    if (transaction.get_problems() != libdnf5::GoalProblem::NO_PROBLEM) {
        throw libdnf5::cli::GoalResolveError(transaction);
    }

    std::set<libdnf5::rpm::Package> resolved_pkgs_set;
    for (const auto & tspkg : transaction.get_transaction_packages()) {
        if (libdnf5::transaction::transaction_item_action_is_inbound(tspkg.get_action())) {
            resolved_pkgs_set.insert(tspkg.get_package());
        }
    }

    // Source RPMs
    if (include_srpms) {
        libdnf5::rpm::PackageQuery source_pkg_query{base};
        source_pkg_query.filter_arch("src");
        source_pkg_query.filter_available();

        std::set<libdnf5::rpm::Package> source_pkgs;
        for (const auto & pkg : resolved_pkgs_set) {
            auto sourcerpm = pkg.get_sourcerpm();
            if (!sourcerpm.empty()) {
                libdnf5::rpm::PackageQuery pkg_query{source_pkg_query};
                pkg_query.filter_epoch({pkg.get_epoch()});

                // Remove ".rpm" to get sourcerpm nevra
                sourcerpm.erase(sourcerpm.length() - 4);
                pkg_query.resolve_pkg_spec(sourcerpm, {}, true);

                if (pkg_query.empty()) {
                    throw libdnf5::RuntimeError(
                        M_("Couldn't find source package {} in available repositories"), sourcerpm);
                } else {
                    pkg_query.filter_latest_evr_any_arch();
                    source_pkgs.insert(*pkg_query.begin());
                }
            } else if (pkg.get_arch() != "src") {
                ctx.print_info(libdnf5::utils::sformat(_("No source rpm defined for package: \"{}\""), pkg.get_name()));
            }
        }
        resolved_pkgs_set.insert(source_pkgs.begin(), source_pkgs.end());
    }

    std::vector<libdnf5::rpm::Package> resolved_pkgs{resolved_pkgs_set.begin(), resolved_pkgs_set.end()};
    return sort_pkgs(std::move(resolved_pkgs));
}

void ManifestSubcommand::add_pkgs_to_manifest(
    libpkgmanifest::manifest::Manifest & manifest,
    libdnf5::Base & base,
    const std::vector<libdnf5::rpm::Package> & pkgs,
    const std::string & arch,
    const bool multiarch) {
    std::optional<std::string> system_repo_id;
    auto repo_sack = base.get_repo_sack();
    if (repo_sack->has_system_repo()) {
        system_repo_id = repo_sack->get_system_repo()->get_id();
    }

    // Add each package and repositories to the manifest
    std::set<std::string> added_repository_ids;
    for (const auto & dnf_pkg : pkgs) {
        const bool is_from_system_repo = dnf_pkg.get_repo_id() == system_repo_id;

        libpkgmanifest::manifest::Package manifest_pkg;
        libpkgmanifest::manifest::Nevra nevra;
        nevra.set_name(dnf_pkg.get_name());
        nevra.set_epoch(dnf_pkg.get_epoch());
        nevra.set_version(dnf_pkg.get_version());
        nevra.set_release(dnf_pkg.get_release());
        nevra.set_arch(dnf_pkg.get_arch());
        manifest_pkg.set_nevra(nevra);
        manifest_pkg.set_size(dnf_pkg.get_download_size());

        libdnf5::repo::RepoQuery rq{base};
        std::string repo_id;
        if (is_from_system_repo) {
            repo_id = dnf_pkg.get_from_repo_id();
        } else {
            repo_id = dnf_pkg.get_repo_id();
        }
        rq.filter_id(repo_id, libdnf5::sack::QueryCmp::EQ);
        if (rq.empty()) {
            throw libdnf5::RuntimeError(
                M_("Package \"{}\" wanted from repository \"{}\", but no such repository was found."),
                dnf_pkg.get_nevra(),
                repo_id);
        }
        libdnf5::repo::RepoWeakPtr dnf_repo = *rq.begin();
        manifest_pkg.set_repo_id(dnf_repo->get_id());

        // Add the package's repository if it has not already been added
        if (!added_repository_ids.contains(manifest_pkg.get_repo_id())) {
            libpkgmanifest::common::Repository manifest_repo;
            manifest_repo.set_id(manifest_pkg.get_repo_id());
            const auto & repo_config = dnf_repo->get_config();
            if (!repo_config.get_metalink_option().empty()) {
                const auto & url = get_arch_generic_url(repo_config.get_metalink_option().get_value(), arch);
                manifest_repo.set_metalink(url);
            } else if (!repo_config.get_mirrorlist_option().empty()) {
                const auto & url = get_arch_generic_url(repo_config.get_mirrorlist_option().get_value(), arch);
                manifest_repo.set_mirrorlist(url);
            } else {
                const auto & baseurls = repo_config.get_baseurl_option().get_value();
                if (baseurls.empty()) {
                    throw libdnf5::RuntimeError(M_("Repository \"{}\" has no baseurl"), dnf_repo->get_id());
                }
                const auto & url = get_arch_generic_url(baseurls.front(), arch);
                manifest_repo.set_baseurl(url);
            }
            manifest.get_repositories().add(manifest_repo);
            added_repository_ids.insert(manifest_repo.get_id());
        }
        manifest_pkg.attach(manifest.get_repositories());

        if (!manifest_pkg.get_repository().get_baseurl().empty()) {
            manifest_pkg.set_location(get_pkg_location(base, dnf_pkg));
        }

        const auto & dnf_checksum = is_from_system_repo ? dnf_pkg.get_hdr_checksum() : dnf_pkg.get_checksum();

        const auto & manifest_checksum_method = checksum_method_rpm_to_manifest(dnf_checksum);
        manifest_pkg.get_checksum().set_method(manifest_checksum_method);
        manifest_pkg.get_checksum().set_digest(dnf_checksum.get_checksum());

        if (multiarch) {
            manifest.get_packages().add(manifest_pkg, arch);
        } else {
            manifest.get_packages().add(manifest_pkg);
        }
    }
}


}  // namespace dnf5
