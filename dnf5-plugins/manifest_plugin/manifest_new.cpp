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

#include <algorithm>
#include <regex>

using namespace libdnf5::cli;

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

std::vector<libdnf5::rpm::Package> sort_pkgs(std::vector<libdnf5::rpm::Package> input) {
    std::sort(input.begin(), input.end(), [](const auto & a, const auto & b) { return a.get_name() < b.get_name(); });
    return input;
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

namespace dnf5 {

void ManifestNewCommand::set_argument_parser() {
    ManifestSubcommand::set_argument_parser();

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description(
        _("Generate a new manifest file using the provided package specs, input file, or system state"));

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
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, true, false, true, false); });
    cmd.register_positional_arg(keys);
}

void ManifestNewCommand::configure() {
    auto & ctx = get_context();

    if (input_path_option->get_priority() > libdnf5::Option::Priority::DEFAULT &&
        !std::filesystem::exists(input_path_option->get_value())) {
        throw CommandExitError(1, M_("Input file \"{}\" does not exist"), input_path_option->get_value());
    }

    if (use_system_option->get_value()) {
        use_system_repository = true;
    }
    if (pkg_specs.empty() && std::filesystem::exists(input_path_option->get_value())) {
        input = libpkgmanifest::input::Parser().parse_prototype(input_path_option->get_value());
    } else if (pkg_specs.empty()) {
        use_system_repository = true;
        generate_system_snapshot = true;
    }

    if (input.has_value()) {
        arches = input->get_archs();
    } else if (arch_option->get_priority() > libdnf5::Option::Priority::DEFAULT) {
        arches = std::vector<std::string>(arch_option->get_value().begin(), arch_option->get_value().end());
    } else {
        arches = {ctx.get_base().get_vars()->get_value("arch")};
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
    if (input.has_value()) {
        create_repos(*base, input->get_repositories());
    } else {
        repo_sack->create_repos_from_system_configuration();

        std::vector<std::pair<std::string, std::string>> repos_from_path{ctx.get_repos_from_path()};
        auto vars = base->get_vars();
        for (auto & id_path_pair : repos_from_path) {
            id_path_pair.first = vars->substitute(id_path_pair.first);
            id_path_pair.second = vars->substitute(id_path_pair.second);
        }
        repo_sack->create_repos_from_paths(repos_from_path, libdnf5::Option::Priority::COMMANDLINE);
    }
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

    std::optional<std::string> system_repo_id;
    if (base->get_repo_sack()->has_system_repo()) {
        system_repo_id = base->get_repo_sack()->get_system_repo()->get_id();
    }

    // Resolve packages
    const auto & resolved_pkgs = resolve_pkgs(*base);

    // Add each package and repositories to the manifest
    std::set<std::string> added_repository_ids;
    for (const auto & dnf_pkg : resolved_pkgs) {
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

        std::optional<libdnf5::repo::RepoWeakPtr> dnf_repo;
        libdnf5::repo::RepoQuery rq{base};
        if (is_from_system_repo) {
            rq.filter_id(dnf_pkg.get_from_repo_id(), libdnf5::sack::QueryCmp::EQ);
        } else {
            rq.filter_id(dnf_pkg.get_repo_id(), libdnf5::sack::QueryCmp::EQ);
        }
        if (rq.size() > 0) {
            dnf_repo = *rq.begin();
            manifest_pkg.set_repo_id((*dnf_repo)->get_id());
        } else {
            manifest_pkg.set_repo_id(BOOTSTRAP_REPO_ID);
        }

        // Add the package's repository if it has not already been added
        if (!added_repository_ids.contains(manifest_pkg.get_repo_id())) {
            libpkgmanifest::common::Repository manifest_repo;
            manifest_repo.set_id(manifest_pkg.get_repo_id());
            if (dnf_repo.has_value()) {
                const auto & repo_config = (*dnf_repo)->get_config();
                if (!repo_config.get_metalink_option().empty()) {
                    const auto & url = get_arch_generic_url(repo_config.get_metalink_option().get_value(), arch);
                    manifest_repo.set_metalink(url);
                } else if (!repo_config.get_mirrorlist_option().empty()) {
                    const auto & url = get_arch_generic_url(repo_config.get_mirrorlist_option().get_value(), arch);
                    manifest_repo.set_mirrorlist(url);
                } else {
                    const auto & baseurls = repo_config.get_baseurl_option().get_value();
                    if (baseurls.empty()) {
                        throw libdnf5::RuntimeError(M_("Repository \"{}\" has no baseurl"), (*dnf_repo)->get_id());
                    }
                    const auto & url = get_arch_generic_url(baseurls.front(), arch);
                    manifest_repo.set_baseurl(url);
                }
            }
            manifest.get_repositories().add(manifest_repo);
            added_repository_ids.insert(manifest_repo.get_id());
        }
        manifest_pkg.attach(manifest.get_repositories());

        if (!manifest_pkg.get_repository().get_baseurl().empty()) {
            manifest_pkg.set_location(get_pkg_location(*base, dnf_pkg));
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

void ManifestNewCommand::run() {
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

std::vector<libdnf5::rpm::Package> ManifestNewCommand::resolve_pkgs(libdnf5::Base & base) {
    auto & ctx = get_context();

    if (generate_system_snapshot) {
        libdnf5::rpm::PackageQuery installed_query{base};
        installed_query.filter_installed();
        std::vector<libdnf5::rpm::Package> installed{installed_query.begin(), installed_query.end()};
        return sort_pkgs(std::move(installed));
    }

    std::set<libdnf5::rpm::Package> resolved_pkgs;
    libdnf5::Goal goal(base);
    if (!pkg_specs.empty()) {
        for (const auto & spec : pkg_specs) {
            goal.add_rpm_install(spec);
        }
    } else if (input.has_value()) {
        goal.set_allow_erasing(input->get_options().get_allow_erasing());
        for (const auto & spec : (*input).get_packages().get_installs()) {
            goal.add_rpm_install(spec);
        }
        for (const auto & spec : (*input).get_packages().get_reinstalls()) {
            goal.add_rpm_reinstall(spec);
        }
    }

    auto transaction = goal.resolve();
    if (transaction.get_problems() != libdnf5::GoalProblem::NO_PROBLEM) {
        throw libdnf5::cli::GoalResolveError(transaction);
    }

    for (const auto & tspkg : transaction.get_transaction_packages()) {
        if (libdnf5::transaction::transaction_item_action_is_inbound(tspkg.get_action())) {
            resolved_pkgs.insert(tspkg.get_package());
        }
    }

    if (srpm_option->get_value()) {
        libdnf5::rpm::PackageQuery source_pkg_query{base};
        source_pkg_query.filter_arch("src");
        source_pkg_query.filter_available();

        std::set<libdnf5::rpm::Package> source_pkgs;
        for (const auto & pkg : resolved_pkgs) {
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

                for (const auto & spkg : pkg_query) {
                    source_pkgs.insert(spkg);
                }
            } else if (pkg.get_arch() != "src") {
                ctx.print_info(libdnf5::utils::sformat(_("No source rpm defined for package: \"{}\""), pkg.get_name()));
            }
        }
        resolved_pkgs.insert(source_pkgs.begin(), source_pkgs.end());
    }

    std::vector<libdnf5::rpm::Package> resolved_pkgs_vector{resolved_pkgs.begin(), resolved_pkgs.end()};
    return sort_pkgs(std::move(resolved_pkgs_vector));
}

}  // namespace dnf5
