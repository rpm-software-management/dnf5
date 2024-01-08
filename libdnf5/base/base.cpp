/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf5/base/base.hpp"

#include "base_impl.hpp"
#include "conf/config.h"
#include "module/module_sack_impl.hpp"
#include "solv/pool.hpp"
#include "utils/dnf4convert/dnf4convert.hpp"
#include "utils/fs/utils.hpp"

#include "libdnf5/conf/config_parser.hpp"
#include "libdnf5/conf/const.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <algorithm>
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

Base::Base(std::vector<std::unique_ptr<Logger>> && loggers)
    : p_impl(new Impl(get_weak_ptr())),
      log_router(std::move(loggers)),
      repo_sack(get_weak_ptr()),
      rpm_package_sack(get_weak_ptr()),
      transaction_history(get_weak_ptr()),
      vars(get_weak_ptr()) {}

Base::~Base() = default;

Base::Impl::Impl(const libdnf5::BaseWeakPtr & base) : rpm_advisory_sack(base), plugins(*base) {}

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
    fs::path conf_file_path{config.get_config_file_path_option().get_value()};
    fs::path conf_dir_path{CONF_DIRECTORY};
    fs::path distribution_conf_dir_path{LIBDNF5_DISTRIBUTION_CONFIG_DIR};

    const auto conf_file_path_priority{config.get_config_file_path_option().get_priority()};
    const bool use_installroot_config{!config.get_use_host_config_option().get_value()};
    const bool user_defined_config_file_name = conf_file_path_priority >= Option::Priority::COMMANDLINE;
    if (use_installroot_config) {
        fs::path installroot_path{config.get_installroot_option().get_value()};
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
        config.load_from_parser(parser, "main", vars, *get_logger());
    }

    // Finally, if a user configuration filename is defined or the file exists in the default location,
    // it will be loaded.
    if (user_defined_config_file_name || fs::exists(conf_file_path)) {
        ConfigParser parser;
        parser.read(conf_file_path);
        config.load_from_parser(parser, "main", vars, *get_logger());
    }
}

void Base::with_config_file_path(std::function<void(const std::string &)> func) {
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

void Base::load_config_from_file() {
    load_config();
}

void Base::load_plugins() {
    const char * plugins_config_dir = std::getenv("LIBDNF_PLUGINS_CONFIG_DIR");
    if (plugins_config_dir && config.get_pluginconfpath_option().get_priority() < Option::Priority::COMMANDLINE) {
        p_impl->plugins.load_plugins(plugins_config_dir);
    } else {
        p_impl->plugins.load_plugins(config.get_pluginconfpath_option().get_value());
    }
}

void Base::setup() {
    auto & pool = p_impl->pool;
    libdnf_user_assert(!pool, "Base was already initialized");

    // Resolve installroot configuration
    std::string vars_installroot{"/"};
    const std::filesystem::path installroot_path{config.get_installroot_option().get_value()};
    if (!config.get_use_host_config_option().get_value()) {
        // Prepend installroot to each reposdir and varsdir
        std::vector<std::string> installroot_reposdirs;
        for (const auto & reposdir : config.get_reposdir_option().get_value()) {
            std::filesystem::path reposdir_path{reposdir};
            installroot_reposdirs.push_back((installroot_path / reposdir_path.relative_path()).string());
        }
        config.get_reposdir_option().set(Option::Priority::INSTALLROOT, installroot_reposdirs);

        // Unless varsdir paths are specified on the command line, load vars
        // from the installroot
        if (config.get_varsdir_option().get_priority() < Option::Priority::COMMANDLINE) {
            vars_installroot = config.get_installroot_option().get_value();
        }
    }
    // Unless the cachedir or logdir are specified on the command line, they
    // should be relative to the installroot
    if (config.get_logdir_option().get_priority() < Option::Priority::COMMANDLINE) {
        const std::filesystem::path logdir_path{config.get_logdir_option().get_value()};
        const auto full_path = installroot_path / logdir_path.relative_path();
        config.get_logdir_option().set(Option::Priority::INSTALLROOT, full_path.string());
    }
    if (config.get_cachedir_option().get_priority() < Option::Priority::COMMANDLINE) {
        const std::filesystem::path cachedir_path{config.get_cachedir_option().get_value()};
        const auto full_path = installroot_path / cachedir_path.relative_path();
        config.get_cachedir_option().set(Option::Priority::INSTALLROOT, full_path.string());
    }
    if (config.get_system_cachedir_option().get_priority() < Option::Priority::COMMANDLINE) {
        const std::filesystem::path system_cachedir_path{config.get_system_cachedir_option().get_value()};
        const auto full_path = installroot_path / system_cachedir_path.relative_path();
        config.get_system_cachedir_option().set(Option::Priority::INSTALLROOT, full_path.string());
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

    // TODO(mblaha) - move system state load closer to the system repo loading
    std::filesystem::path system_state_dir{config.get_system_state_dir_option().get_value()};
    p_impl->system_state.emplace(installroot.get_value() / system_state_dir.relative_path());

    auto & system_state = p_impl->get_system_state();

    // TODO(mblaha) - this is temporary override of modules state by reading
    // dnf4 persistor from /etc/dnf/modules.d/
    // Remove once reading of dnf4 data is not needed
    libdnf5::dnf4convert::Dnf4Convert convertor(get_weak_ptr());
    if ((!std::filesystem::exists(system_state.get_module_state_path()))) {
        system_state.reset_module_states(convertor.read_module_states());
    }

    if (system_state.packages_import_required()) {
        // TODO(mblaha) - first try dnf5 history database, then fall back to dnf4
        std::map<std::string, libdnf5::system::PackageState> package_states;
        std::map<std::string, libdnf5::system::NevraState> nevra_states;
        std::map<std::string, libdnf5::system::GroupState> group_states;
        std::map<std::string, libdnf5::system::EnvironmentState> environment_states;

        if (convertor.read_package_states_from_history(
                package_states, nevra_states, group_states, environment_states)) {
            system_state.reset_packages_states(
                std::move(package_states),
                std::move(nevra_states),
                std::move(group_states),
                std::move(environment_states));
        }
    }

    config.get_varsdir_option().lock("Locked by Base::setup()");
    pool_setdisttype(**pool, DISTTYPE_RPM);
    // TODO(jmracek) - architecture variable is changable therefore architecture in vars must be synchronized with RpmPool
    // (and force to recompute provides) or locked
    const char * arch = vars->get_value("arch").c_str();
    pool_setarch(**pool, arch);
    module_sack.p_impl->set_arch(arch);
    pool_set_rootdir(**pool, installroot.get_value().c_str());

    p_impl->plugins.post_base_setup();
}

bool Base::is_initialized() {
    return p_impl->pool.get() != nullptr;
}

}  // namespace libdnf5
