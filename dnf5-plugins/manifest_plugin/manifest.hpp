// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_PLUGINS_MANIFEST_PLUGIN_MANIFEST_HPP
#define DNF5_PLUGINS_MANIFEST_PLUGIN_MANIFEST_HPP

#include <dnf5/context.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <libpkgmanifest/common/repositories.hpp>
#include <libpkgmanifest/input/input.hpp>
#include <libpkgmanifest/manifest/manifest.hpp>

#include <vector>

const std::string DEFAULT_INPUT_FILENAME{"rpms.in.yaml"};
const std::string DEFAULT_MANIFEST_FILENAME{"packages.manifest.yaml"};
const std::string MODULE_FILENAME{"modules_dump.modulemd.yaml"};
const std::string MODULAR_DATA_SEPARATOR{"..."};
const std::string BOOTSTRAP_REPO_ID{"bootstrap"};

libdnf5::rpm::Nevra nevra_manifest_to_dnf(const libpkgmanifest::manifest::Nevra & manifest_nevra);

std::filesystem::path get_manifest_path(libdnf5::OptionPath & option, const std::string & arch);

void set_repo_callbacks(libdnf5::Base & base);

namespace dnf5 {

class ManifestCommand : public Command {
public:
    explicit ManifestCommand(Context & context) : Command(context, "manifest") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};


class ManifestSubcommand : public Command {
public:
    explicit ManifestSubcommand(Context & context, const std::string & name) : Command(context, name) {}
    void set_argument_parser() override;
    void pre_configure() override;

protected:
    void create_repos(libdnf5::Base & base, libpkgmanifest::common::Repositories manifest_repos) const;
    std::unique_ptr<libdnf5::Base> create_base_for_arch(const std::string & arch) const;

    libdnf5::OptionPath * manifest_path_option{nullptr};
    std::unique_ptr<libdnf5::ConfigMain> config_before_setup;
};

class ManifestNewCommand : public ManifestSubcommand {
public:
    explicit ManifestNewCommand(Context & context) : ManifestSubcommand(context, "new") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    void populate_manifest(libpkgmanifest::manifest::Manifest &, const std::string & arch, const bool multiarch);
    std::vector<libdnf5::rpm::Package> resolve_pkgs(libdnf5::Base & base);

    libdnf5::OptionPath * input_path_option{nullptr};
    libdnf5::OptionBool * use_system_option{nullptr};
    libdnf5::OptionBool * per_arch_option{nullptr};
    libdnf5::OptionStringList * arch_option{nullptr};
    libdnf5::OptionBool * srpm_option{nullptr};

    std::vector<std::string> pkg_specs;
    std::optional<libpkgmanifest::input::Input> input;
    std::vector<std::string> arches;
    bool use_system_repository{false};
    bool generate_system_snapshot{false};
};

class ManifestDownloadCommand : public ManifestSubcommand {
public:
    explicit ManifestDownloadCommand(Context & context) : ManifestSubcommand(context, "download") {}
    void set_argument_parser() override;
    void pre_configure() override;
    void configure() override;
    void run() override;

private:
    void download_packages(
        libpkgmanifest::manifest::Manifest & manifest,
        const std::string & arch,
        const std::filesystem::path & default_destdir);

    libdnf5::OptionStringList * arch_option{nullptr};
    libdnf5::OptionBool * srpm_option{nullptr};

    std::vector<std::string> arches;
    std::map<std::string, std::filesystem::path> manifest_paths;
};

class ManifestInstallCommand : public ManifestSubcommand {
public:
    explicit ManifestInstallCommand(Context & context) : ManifestSubcommand(context, "install") {}
    void set_argument_parser() override;
    void pre_configure() override;
    void configure() override;
    void run() override;

private:
    libpkgmanifest::manifest::Manifest manifest;
};

}  // namespace dnf5

#endif  // DNF5_PLUGINS_MANIFEST_PLUGIN_MANIFEST_HPP
