// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5/base/base.hpp"

#include "../conf/config_utils.hpp"
#include "../plugin/plugin_info_impl.hpp"
#include "base_impl.hpp"
#include "conf/config.h"

#ifdef WITH_MODULEMD
#include "module/module_sack_impl.hpp"

#include "libdnf5/module/module_sack.hpp"
#endif

#include "comps/comps_sack_impl.hpp"
#include "solv/pool.hpp"
#include "utils/dnf4convert/dnf4convert.hpp"
#include "utils/fs/utils.hpp"

#include "libdnf5/comps/comps_sack.hpp"
#include "libdnf5/conf/config_parser.hpp"
#include "libdnf5/conf/const.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace libdnf5 {

static std::atomic<Base *> locked_base{nullptr};
static std::mutex locked_base_mutex;

Base::Base(std::vector<std::unique_ptr<Logger>> && loggers) : p_impl(new Impl(get_weak_ptr(), std::move(loggers))) {}

Base::~Base() = default;

Base::Impl::Impl(const libdnf5::BaseWeakPtr & base, std::vector<std::unique_ptr<Logger>> && loggers)
    : rpm_advisory_sack(base),
      plugins(*base),
      log_router(std::move(loggers)),
      comps_sack(base),
      repo_sack(base),
      rpm_package_sack(base),
#ifdef WITH_MODULEMD
      module_sack(base),
#endif
      transaction_history(base),
      vars(base) {
}

void Base::lock() {
    locked_base_mutex.lock();
    locked_base = this;
}

void Base::unlock() {
    libdnf_user_assert(locked_base, "Base::unlock() called on unlocked \"Base\" instance");
    libdnf_user_assert(
        locked_base == this, "Called Base::unlock(). But the lock is not owned by this \"Base\" instance.");
    locked_base = nullptr;
    locked_base_mutex.unlock();
}

Base * Base::get_locked_base() noexcept {
    return locked_base;
}

void Base::load_config() {
    fs::path conf_file_path{p_impl->config.get_config_file_path_option().get_value()};
    fs::path conf_dir_path{CONF_DIRECTORY};
    fs::path distribution_conf_dir_path{LIBDNF5_DISTRIBUTION_CONFIG_DIR};

    const auto conf_file_path_priority{p_impl->config.get_config_file_path_option().get_priority()};
    const bool use_installroot_config{!p_impl->config.get_use_host_config_option().get_value()};
    const bool user_defined_config_file_name = conf_file_path_priority >= Option::Priority::COMMANDLINE;
    if (use_installroot_config) {
        fs::path installroot_path{p_impl->config.get_installroot_option().get_value()};
        if (!user_defined_config_file_name) {
            conf_file_path = installroot_path / conf_file_path.relative_path();
        }
        conf_dir_path = installroot_path / conf_dir_path.relative_path();
        distribution_conf_dir_path = installroot_path / distribution_conf_dir_path.relative_path();
    }

    // Loads configuration from drop-in directories
    const auto paths = utils::fs::create_sorted_file_list({conf_dir_path, distribution_conf_dir_path}, ".conf");
    for (const auto & path : paths) {
        ConfigParser parser;
        parser.read(path);
        p_impl->config.load_from_parser(parser, "main", p_impl->vars, *get_logger());
    }

    // Finally, if a user configuration filename is defined or the file exists in the default location,
    // it will be loaded.
    if (user_defined_config_file_name || fs::exists(conf_file_path)) {
        ConfigParser parser;
        parser.read(conf_file_path);
        p_impl->config.load_from_parser(parser, "main", p_impl->vars, *get_logger());
    }
}

void Base::Impl::with_config_file_path(std::function<void(const std::string &)> func) {
    std::filesystem::path conf_path{config.get_config_file_path_option().get_value()};
    const auto & conf_path_priority = config.get_config_file_path_option().get_priority();
    const auto & use_host_config = config.get_use_host_config_option().get_value();
    if (!use_host_config && conf_path_priority < Option::Priority::COMMANDLINE) {
        conf_path = config.get_installroot_option().get_value() / conf_path.relative_path();
    }
    try {
        return func(conf_path.string());
    } catch (const MissingConfigError & e) {
        // Ignore the missing config file unless the user specified it via --config=...
        if (conf_path_priority >= libdnf5::Option::Priority::COMMANDLINE) {
            throw;
        }
    } catch (const InaccessibleConfigError & e) {
        // Ignore the inaccessible config file unless the user specified it via --config=...
        if (conf_path_priority >= libdnf5::Option::Priority::COMMANDLINE) {
            throw;
        }
    }
}

std::tuple<std::string, libdnf5::ConfigParser, bool> Base::load_plugin_config(const std::string & config_file_path) {
    auto & logger = *get_logger();

    libdnf5::ConfigParser parser;
    parser.read(config_file_path);

    std::string plugin_name;
    try {
        plugin_name = parser.get_value("main", "name");
    } catch (const ConfigParserError &) {
        plugin_name = std::filesystem::path(config_file_path).stem();
        logger.warning(
            "Missing plugin name in configuration file \"{}\". \"{}\" will be used.", config_file_path, plugin_name);
    }

    bool is_enabled;
    bool is_enabled_set{false};
    for (auto it = p_impl->plugins_enablement.rbegin(); it != p_impl->plugins_enablement.rend(); ++it) {
        if (sack::match_string(plugin_name, sack::QueryCmp::GLOB, it->first)) {
            is_enabled = it->second;
            is_enabled_set = true;
            break;
        }
    }
    if (!is_enabled_set) {
        enum class Enabled { NO, YES, HOST_ONLY, INSTALLROOT_ONLY } enabled;
        const auto & enabled_str = parser.get_value("main", "enabled");
        if (enabled_str == "host-only") {
            enabled = Enabled::HOST_ONLY;
        } else if (enabled_str == "installroot-only") {
            enabled = Enabled::INSTALLROOT_ONLY;
        } else {
            try {
                enabled = OptionBool(false).from_string(enabled_str) ? Enabled::YES : Enabled::NO;
            } catch (OptionInvalidValueError & ex) {
                throw OptionInvalidValueError(M_("Invalid option value: enabled={}"), enabled_str);
            }
        }
        const auto & installroot = get_config().get_installroot_option().get_value();
        is_enabled = enabled == Enabled::YES || (enabled == Enabled::HOST_ONLY && installroot == "/") ||
                     (enabled == Enabled::INSTALLROOT_ONLY && installroot != "/");
    }

    if (!is_enabled) {
        logger.debug("Skip disabled plugin \"{}\"", config_file_path);
        // Creates a PluginInfo for the unloaded plugin.
        auto & plugins_info = InternalBaseUser::get_plugins_info(this);
        plugins_info.emplace_back(libdnf5::plugin::PluginInfo::Impl::create_plugin_info(plugin_name, nullptr));
    }

    return {plugin_name, parser, is_enabled};
}

void Base::load_plugins() {
    // load plugins according to configuration
    if (!p_impl->config.get_plugins_option().get_value()) {
        return;
    }

    const char * plugins_config_dir = std::getenv("LIBDNF_PLUGINS_CONFIG_DIR");
    if (plugins_config_dir &&
        p_impl->config.get_pluginconfpath_option().get_priority() < Option::Priority::COMMANDLINE) {
        p_impl->plugins.load_plugins(plugins_config_dir);
    } else {
        p_impl->plugins.load_plugins(p_impl->config.get_pluginconfpath_option().get_value());
    }
}

void Base::add_plugin(
    const std::string & plugin_name,
    libdnf5::ConfigParser && plugin_config,
    libdnf5::plugin::IPlugin & iplugin_instance) {
    auto plugin = std::make_unique<libdnf5::plugin::Plugin>(iplugin_instance, std::move(plugin_config));
    p_impl->plugins.register_plugin(std::move(plugin));

    auto & plugins_info = InternalBaseUser::get_plugins_info(this);
    plugins_info.emplace_back(plugin::PluginInfo::Impl::create_plugin_info(plugin_name, &iplugin_instance));
}

void Base::enable_disable_plugins(const std::vector<std::string> & plugin_names, bool enable) {
    libdnf_user_assert(!p_impl->pool, "Base::enable_disable_plugins must be called before Base::setup");

    for (const auto & plugin_name : plugin_names) {
        p_impl->plugins_enablement.erase(plugin_name);
        p_impl->plugins_enablement.insert({plugin_name, enable});
    }
}

const std::vector<plugin::PluginInfo> & Base::get_plugins_info() const {
    libdnf_user_assert(p_impl->pool, "Base::get_plugins_info must not be called before Base::setup");
    return p_impl->get_plugins_info();
}

void Base::setup() {
    auto & pool = p_impl->pool;
    libdnf_user_assert(!pool, "Base was already initialized");

    // Resolve installroot configuration
    std::string vars_installroot{"/"};
    const std::filesystem::path installroot_path{p_impl->config.get_installroot_option().get_value()};
    if (!p_impl->config.get_use_host_config_option().get_value()) {
        // Prepend installroot to each reposdir and varsdir
        std::vector<std::string> installroot_reposdirs;
        for (const auto & reposdir : p_impl->config.get_reposdir_option().get_value()) {
            std::filesystem::path reposdir_path{reposdir};
            installroot_reposdirs.push_back((installroot_path / reposdir_path.relative_path()).string());
        }
        p_impl->config.get_reposdir_option().set(Option::Priority::INSTALLROOT, installroot_reposdirs);

        // Unless varsdir paths are specified on the command line, load vars
        // from the installroot
        if (p_impl->config.get_varsdir_option().get_priority() < Option::Priority::COMMANDLINE) {
            vars_installroot = p_impl->config.get_installroot_option().get_value();
        }
    }
    // Unless the cachedir or logdir are specified on the command line, they
    // should be relative to the installroot
    if (p_impl->config.get_logdir_option().get_priority() < Option::Priority::COMMANDLINE) {
        const std::filesystem::path logdir_path{p_impl->config.get_logdir_option().get_value()};
        const auto full_path = installroot_path / logdir_path.relative_path();
        p_impl->config.get_logdir_option().set(Option::Priority::INSTALLROOT, full_path.string());
    }
    if (p_impl->config.get_cachedir_option().get_priority() < Option::Priority::COMMANDLINE) {
        const std::filesystem::path cachedir_path{p_impl->config.get_cachedir_option().get_value()};
        const auto full_path = installroot_path / cachedir_path.relative_path();
        p_impl->config.get_cachedir_option().set(Option::Priority::INSTALLROOT, full_path.string());
    }
    if (p_impl->config.get_system_cachedir_option().get_priority() < Option::Priority::COMMANDLINE) {
        const std::filesystem::path system_cachedir_path{p_impl->config.get_system_cachedir_option().get_value()};
        const auto full_path = installroot_path / system_cachedir_path.relative_path();
        p_impl->config.get_system_cachedir_option().set(Option::Priority::INSTALLROOT, full_path.string());
    }

    // Add protected packages from files from installroot
    {
        auto & protected_option = p_impl->config.get_protected_packages_option();
        auto resolved_protected_packages = resolve_path_globs(protected_option.get_value_string(), installroot_path);
        protected_option.set(protected_option.get_priority(), resolved_protected_packages);
    }

    load_plugins();
    p_impl->plugins.init();

    p_impl->plugins.pre_base_setup();

    pool.reset(new libdnf5::solv::RpmPool);
    p_impl->comps_pool.reset(new libdnf5::solv::CompsPool);
    auto & config = get_config();
    auto & installroot = config.get_installroot_option();
    installroot.lock("Locked by Base::setup()");
    auto vars = get_vars();

    vars->load(vars_installroot, config.get_varsdir_option().get_value());

    // Load vendor change policies
    fs::path vendor_conf_dir_path{VENDOR_CONF_DIR};
    fs::path distribution_vendor_conf_dir_path{LIBDNF5_DISTRIBUTION_VENDOR_CONF_DIR};
    const bool use_installroot_config{!p_impl->config.get_use_host_config_option().get_value()};
    if (use_installroot_config) {
        fs::path installroot_path{p_impl->config.get_installroot_option().get_value()};
        vendor_conf_dir_path = installroot_path / vendor_conf_dir_path.relative_path();
        distribution_vendor_conf_dir_path = installroot_path / distribution_vendor_conf_dir_path.relative_path();
    }
    const auto paths =
        utils::fs::create_sorted_file_list({vendor_conf_dir_path, distribution_vendor_conf_dir_path}, ".conf");
    for (const auto & path : paths) {
        pool->load_vendor_change_policy(path);
    }

    config.get_varsdir_option().lock("Locked by Base::setup()");
    pool_setdisttype(**pool, DISTTYPE_RPM);
    // TODO(jmracek) - architecture variable is changeable therefore architecture in vars must be synchronized with RpmPool
    // (and force to recompute provides) or locked
    const char * arch = vars->get_value("arch").c_str();
    pool_setarch(**pool, arch);
#ifdef WITH_MODULEMD
    p_impl->module_sack.p_impl->set_arch(arch);
#endif
    pool_set_rootdir(**pool, installroot.get_value().c_str());

    p_impl->plugins.post_base_setup();
}

bool Base::lock_system_repo(libdnf5::utils::LockAccess access, libdnf5::utils::LockBlocking blocking) {
    return p_impl->lock_system_repo(access, blocking);
}

void Base::unlock_system_repo() {
    p_impl->unlock_system_repo();
}

const libdnf5::utils::Locker * Base::get_system_repo_lock() const noexcept {
    return p_impl->get_system_repo_lock();
}

bool Base::is_initialized() {
    return p_impl->pool.get() != nullptr;
}

void Base::notify_repos_configured() {
    auto & pool = p_impl->pool;
    libdnf_user_assert(pool, "Base has not been initialized");

    auto & repos_configured = p_impl->repos_configured;
    libdnf_user_assert(!repos_configured, "The `notify_repos_configured` notification has already been called");

    p_impl->plugins.repos_configured();

    repos_configured = true;
}

bool Base::are_repos_configured() const noexcept {
    return p_impl->repos_configured;
}

ConfigMain & Base::get_config() {
    return p_impl->config;
}
LogRouterWeakPtr Base::get_logger() {
    return {&p_impl->log_router, &p_impl->log_router_guard};
}
comps::CompsSackWeakPtr Base::get_comps_sack() {
    return p_impl->comps_sack.get_weak_ptr();
}
repo::RepoSackWeakPtr Base::get_repo_sack() {
    return p_impl->repo_sack.get_weak_ptr();
}
rpm::PackageSackWeakPtr Base::get_rpm_package_sack() {
    return p_impl->rpm_package_sack.get_weak_ptr();
}

transaction::TransactionHistoryWeakPtr Base::get_transaction_history() {
    return p_impl->transaction_history.get_weak_ptr();
}
libdnf5::module::ModuleSackWeakPtr Base::get_module_sack() {
#ifdef WITH_MODULEMD
    return p_impl->module_sack.get_weak_ptr();
#else
    libdnf_throw_assertion("libdnf5 compiled without module support.");
#endif
}

VarsWeakPtr Base::get_vars() {
    return {&p_impl->vars, &p_impl->vars_guard};
}

libdnf5::BaseWeakPtr Base::get_weak_ptr() {
    return {this, &base_guard};
}

void Base::set_download_callbacks(std::unique_ptr<repo::DownloadCallbacks> && download_callbacks) {
    this->p_impl->download_callbacks = std::move(download_callbacks);
}
repo::DownloadCallbacks * Base::get_download_callbacks() {
    return p_impl->download_callbacks.get();
}

libdnf5::system::State & Base::Impl::get_system_state() {
    if (system_state) {
        return *system_state;
    }

    std::filesystem::path system_state_dir{config.get_system_state_dir_option().get_value()};
    // Getting the base from repo_sack is not ideal
    system_state.emplace(
        repo_sack.get_base(), config.get_installroot_option().get_value() / system_state_dir.relative_path());

    // TODO(mblaha) - this is temporary override of modules state by reading
    // dnf4 persistor from /etc/dnf/modules.d/
    // Remove once reading of dnf4 data is not needed
    libdnf5::dnf4convert::Dnf4Convert convertor(repo_sack.get_base());
#ifdef WITH_MODULEMD
    if ((!std::filesystem::exists(system_state->get_module_state_path()))) {
        system_state->reset_module_states(convertor.read_module_states());
    }
#endif

    if (system_state->packages_import_required()) {
        // TODO(mblaha) - first try dnf5 history database, then fall back to dnf4
        std::map<std::string, libdnf5::system::PackageState> package_states;
        std::map<std::string, libdnf5::system::NevraState> nevra_states;
        std::map<std::string, libdnf5::system::GroupState> group_states;
        std::map<std::string, libdnf5::system::EnvironmentState> environment_states;

        if (convertor.read_package_states_from_history(
                package_states, nevra_states, group_states, environment_states)) {
            system_state->reset_packages_states(
                std::move(package_states),
                std::move(nevra_states),
                std::move(group_states),
                std::move(environment_states));
        }
    }

    return *system_state;
}

bool Base::Impl::lock_system_repo(libdnf5::utils::LockAccess access, libdnf5::utils::LockBlocking blocking) {
    if (!system_repo_lock.has_value()) {
        auto & installroot_option = config.get_installroot_option();
        installroot_option.lock("Locked by Base::Impl::lock_system_repo()");
        const auto & system_state_dir = config.get_system_state_dir_option().get_value();

        const auto & relative_path =
            std::filesystem::path{system_state_dir}.relative_path() / std::filesystem::path{SYSTEM_REPO_LOCK_FILENAME};
        const auto & lock_file_path = installroot_option.get_value() / relative_path;

        std::filesystem::create_directories(lock_file_path.parent_path());
        system_repo_lock = libdnf5::utils::Locker{lock_file_path, true};
    }
    return system_repo_lock->lock(access, blocking);
}

void Base::Impl::unlock_system_repo() {
    if (system_repo_lock.has_value()) {
        system_repo_lock->unlock();
    }
}

const libdnf5::utils::Locker * Base::Impl::get_system_repo_lock() const noexcept {
    return system_repo_lock ? std::to_address(system_repo_lock) : nullptr;
}

}  // namespace libdnf5
