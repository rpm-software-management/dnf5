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

#include "libdnf/base/base.hpp"

#include "base_impl.hpp"
#include "conf/config.h"
#include "solv/pool.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/dnf4convert/dnf4convert.hpp"

#include "libdnf/conf/config_parser.hpp"
#include "libdnf/conf/const.hpp"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <vector>

namespace libdnf {

static std::atomic<Base *> locked_base{nullptr};
static std::mutex locked_base_mutex;

Base::Base(std::vector<std::unique_ptr<Logger>> && loggers)
    : p_impl(new Impl(get_weak_ptr())),
      log_router(std::move(loggers)),
      repo_sack(get_weak_ptr()),
      rpm_package_sack(get_weak_ptr()),
      transaction_history(get_weak_ptr()),
      vars(get_weak_ptr()) {
    load_defaults();
}

Base::~Base() = default;

Base::Impl::Impl(const libdnf::BaseWeakPtr & base) : rpm_advisory_sack(base) {}

void Base::lock() {
    locked_base_mutex.lock();
    locked_base = this;
}

void Base::unlock() {
    libdnf_assert(locked_base, "Base::unlock() called on unlocked \"Base\" instance");
    libdnf_assert(locked_base == this, "Called Base::unlock(). But the lock is not owned by this \"Base\" instance.");
    locked_base = nullptr;
    locked_base_mutex.unlock();
}

Base * Base::get_locked_base() noexcept {
    return locked_base;
}

void Base::load_defaults() {
    const std::string file_path = LIBDNF5_DISTRIBUTION_CONFIG_FILE;
    try {
        log_router.debug("Loading default configuration from \"{}\"", file_path);
        ConfigParser parser;
        parser.read(file_path);
        config.load_from_parser(parser, "main", vars, *get_logger(), Option::Priority::DEFAULT);
    } catch (const std::filesystem::filesystem_error & ex) {
        if (ex.code().value() == ENOENT) {
            log_router.debug("Configuration file \"{}\" not found", file_path);
        } else {
            std::throw_with_nested(RuntimeError(M_("Unable to load configuration file \"{}\""), file_path));
        }
    } catch (const libdnf::Error & ex) {
        std::throw_with_nested(RuntimeError(M_("Error in configuration file \"{}\""), file_path));
    }
}

void Base::load_config_from_file(const std::string & path) try {
    ConfigParser parser;
    parser.read(path);
    config.load_from_parser(parser, "main", vars, *get_logger());
} catch (const Error & e) {
    std::throw_with_nested(RuntimeError(M_("Unable to load configuration file \"{}\""), path));
}

void Base::load_config_from_file() {
    load_config_from_file(config.config_file_path().get_value());
}

void Base::load_config_from_dir(const std::string & dir_path) {
    std::vector<std::filesystem::path> paths;
    for (auto & dentry : std::filesystem::directory_iterator(dir_path)) {
        auto & path = dentry.path();
        if (path.extension() == ".conf") {
            paths.push_back(path);
        }
    }
    std::sort(paths.begin(), paths.end());
    for (auto & path : paths) {
        load_config_from_file(path);
    }
}

void Base::load_config_from_dir() {
    load_config_from_dir(libdnf::CONF_DIRECTORY);
}

void Base::add_plugin(plugin::IPlugin & iplugin_instance) {
    plugins.register_plugin(std::make_unique<plugin::Plugin>(iplugin_instance));
}

void Base::load_plugins() {
    const char * plugins_config_dir = std::getenv("LIBDNF_PLUGINS_CONFIG_DIR");
    if (plugins_config_dir && config.pluginconfpath().get_priority() < Option::Priority::COMMANDLINE) {
        plugins.load_plugins(plugins_config_dir);
    } else {
        plugins.load_plugins(config.pluginconfpath().get_value());
    }
}

void Base::setup() {
    auto & pool = p_impl->pool;
    libdnf_assert(!pool, "Base was already initialized");

    load_plugins();
    plugins.init();

    plugins.pre_base_setup();

    pool.reset(new libdnf::solv::Pool);
    auto & config = get_config();
    auto & installroot = config.installroot();
    installroot.lock("Locked by Base::setup()");

    get_vars()->load(installroot.get_value(), config.varsdir().get_value());

    // TODO(mblaha) - move system state load closer to the system repo loading
    std::filesystem::path system_state_dir{config.system_state_dir().get_value()};
    p_impl->system_state.emplace(installroot.get_value() / system_state_dir.relative_path());

    auto & system_state = p_impl->get_system_state();

    // TODO(mblaha) - this is temporary override of modules state by reading
    // dnf4 persistor from /etc/dnf/modules.d/
    // Remove once reading of dnf4 data is not needed
    libdnf::dnf4convert::Dnf4Convert convertor(get_weak_ptr());
    system_state.reset_module_states(convertor.read_module_states());

    if (system_state.packages_import_required()) {
        // TODO(mblaha) - first try dnf5 history database, then fall back to dnf4
        std::map<std::string, libdnf::system::PackageState> package_states;
        std::map<std::string, libdnf::system::NevraState> nevra_states;
        std::map<std::string, libdnf::system::GroupState> group_states;
        std::map<std::string, libdnf::system::EnvironmentState> environment_states;

        if (convertor.read_package_states_from_history(
                package_states, nevra_states, group_states, environment_states)) {
            system_state.reset_packages_states(
                std::move(package_states),
                std::move(nevra_states),
                std::move(group_states),
                std::move(environment_states));
        }
    }

    config.varsdir().lock("Locked by Base::setup()");
    pool_setdisttype(**pool, DISTTYPE_RPM);
    // TODO(jmracek) - architecture variable is changable therefore architecture in vars must be synchronized with Pool
    // (and force to recompute provides) or locked
    pool_setarch(**pool, get_vars()->get_value("arch").c_str());
    pool_set_rootdir(**pool, installroot.get_value().c_str());

    plugins.post_base_setup();
}

}  // namespace libdnf
