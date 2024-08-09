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

#include "libdnf5/conf/config_main.hpp"

#include "config.h"
#include "config_utils.hpp"
#include "utils/system.hpp"

#include "libdnf5/common/xdg.hpp"
#include "libdnf5/conf/config_parser.hpp"
#include "libdnf5/conf/const.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/file.hpp"
#include "libdnf5/utils/os_release.hpp"

#include <glob.h>
#include <libdnf5/rpm/arch.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>
#include <utility>

namespace libdnf5 {

/// @brief Converts a friendly bandwidth option to bytes
///
/// Function converts a friendly bandwidth option to bytes.  The input
/// should be a string containing a (possibly floating point)
/// number followed by an optional single character unit. Valid
/// units are 'k', 'M', 'G'. Case is ignored. The convention that
/// 1k = 1024 bytes is used.
///
/// @param str Bandwidth as user friendly string
/// @return int Number of bytes
static int str_to_bytes(const std::string & str) {
    if (str.empty()) {
        throw OptionInvalidValueError(M_("Input is empty. Must contain a value."));
    }

    std::size_t idx;
    auto res = std::stod(str, &idx);
    if (res < 0) {
        throw OptionInvalidValueError(M_("Input value '{}' must not be negative"), str);
    }

    if (idx < str.length()) {
        if (idx < str.length() - 1) {
            throw OptionInvalidValueError(M_("Could not convert '{}' to bytes"), str);
        }
        switch (str.back()) {
            case 'k':
            case 'K':
                res *= 1024;
                break;
            case 'm':
            case 'M':
                res *= 1024 * 1024;
                break;
            case 'g':
            case 'G':
                res *= 1024 * 1024 * 1024;
                break;
            default:
                throw OptionInvalidValueError(M_("Unknown unit '{}'"), str.back());
        }
    }

    return static_cast<int>(res);
}

static std::string get_user_agent() {
    utils::OSRelease os_release;
    return std::format("libdnf ({} {}; {}; {}.{})",
        os_release.get_value("NAME"),
        os_release.get_value("VERSION_ID"),
        os_release.get_value("VARIANT_ID"),
        utils::get_os(),
        rpm::get_base_arch(utils::detect_arch()));
}

class ConfigMain::Impl {
    friend class ConfigMain;

    explicit Impl(Config & owner);

    Config & owner;

    OptionNumber<std::int32_t> debuglevel{2, 0, 10};
    OptionNumber<std::int32_t> errorlevel{3, 0, 10};
    OptionPath installroot{"/", false, true};
    OptionBool use_host_config{false};
    OptionPath config_file_path{CONF_FILENAME};
    OptionBool plugins{true};
    OptionPath pluginpath{DEFAULT_LIBDNF5_PLUGINS_LIB_DIR};
    OptionPath pluginconfpath{PLUGINS_CONF_DIR};
    OptionPath persistdir{PERSISTDIR};
    OptionPath system_state_dir{SYSTEM_STATE_DIR};
    OptionPath transaction_history_dir{SYSTEM_STATE_DIR};
    OptionBool transformdb{true};
    OptionNumber<std::int32_t> recent{7, 0};
    OptionBool reset_nice{true};
    OptionPath system_cachedir{SYSTEM_CACHEDIR};
    OptionEnum cacheonly{"none", {"all", "metadata", "none"}};
    OptionBool keepcache{false};
    OptionPath logdir{geteuid() == 0 ? "/var/log" : libdnf5::xdg::get_user_state_dir()};
    OptionNumber<std::int32_t> log_size{1024 * 1024, str_to_bytes};
    OptionNumber<std::int32_t> log_rotate{4, 0};
    OptionPath debugdir{"./debugdata"};
    OptionStringList varsdir{VARS_DIRS};
    OptionStringList reposdir{REPOSITORY_CONF_DIRS};
    OptionBool debug_solver{false};
    OptionStringAppendList installonlypkgs{INSTALLONLYPKGS};
    OptionStringList group_package_types{GROUP_PACKAGE_TYPES};
    OptionStringAppendSet optional_metadata_types{
        OptionStringAppendSet::ValueType{libdnf5::METADATA_TYPE_COMPS, libdnf5::METADATA_TYPE_UPDATEINFO}};
    OptionNumber<std::uint32_t> installonly_limit{3, 0, [](const std::string & value) -> std::uint32_t {
                                                      if (value == "<off>") {
                                                          return 0;
                                                      }
                                                      try {
                                                          return static_cast<std::uint32_t>(std::stoul(value));
                                                      } catch (...) {
                                                          return 0;
                                                      }
                                                  }};

    OptionStringAppendList tsflags{std::vector<std::string>{}};
    OptionBool assumeyes{false};
    OptionBool assumeno{false};
    OptionBool check_config_file_age{true};
    OptionBool defaultyes{false};
    OptionBool diskspacecheck{true};
    OptionBool localpkg_gpgcheck{false};
    OptionBool gpgkey_dns_verification{false};
    OptionBool obsoletes{true};
    OptionBool exit_on_lock{false};
    OptionBool allow_vendor_change{true};
    OptionSeconds metadata_timer_sync{60 * 60 * 3};  // 3 hours
    OptionStringList disable_excludes{std::vector<std::string>{}};
    OptionEnum multilib_policy{"best", {"best", "all"}};  // :api
    OptionBool best{true};                                // :api
    OptionBool install_weak_deps{true};
    OptionBool allow_downgrade{true};
    OptionString bugtracker_url{BUGTRACKER};
    OptionBool zchunk{true};

    OptionEnum color{"auto", {"auto", "never", "always"}, [](const std::string & value) {
                         const std::array<const char *, 4> always{{"on", "yes", "1", "true"}};
                         const std::array<const char *, 4> never{{"off", "no", "0", "false"}};
                         const std::array<const char *, 2> aut{{"tty", "if-tty"}};
                         std::string tmp;
                         if (std::find(always.begin(), always.end(), value) != always.end()) {
                             tmp = "always";
                         } else if (std::find(never.begin(), never.end(), value) != never.end()) {
                             tmp = "never";
                         } else if (std::find(aut.begin(), aut.end(), value) != aut.end()) {
                             tmp = "auto";
                         } else {
                             tmp = value;
                         }
                         return tmp;
                     }};

    OptionString color_list_installed_older{"yellow"};
    OptionString color_list_installed_newer{"bold,yellow"};
    OptionString color_list_installed_reinstall{"dim,cyan"};
    OptionString color_list_installed_extra{"bold,red"};
    OptionString color_list_available_upgrade{"bold,blue"};
    OptionString color_list_available_downgrade{"dim,magenta"};
    OptionString color_list_available_reinstall{"bold,green"};
    OptionString color_list_available_install{"bold,cyan"};
    OptionString color_update_installed{"dim,red"};
    OptionString color_update_local{"dim,green"};
    OptionString color_update_remote{"bold,green"};
    OptionString color_search_match{"bold,magenta"};
    OptionBool history_record{true};
    OptionStringList history_record_packages{std::vector<std::string>{"dnf", "rpm"}};
    OptionString rpmverbosity{"info"};
    OptionBool strict{true};  // deprecated
    OptionBool skip_broken{false};
    OptionBool skip_unavailable{false};
    OptionBool autocheck_running_kernel{true};  // :yum-compatibility
    OptionBool clean_requirements_on_remove{true};

    OptionEnum history_list_view{
        "commands", {"single-user-commands", "users", "commands"}, [](const std::string & value) {
            if (value == "cmds" || value == "default") {
                return std::string("commands");
            }
            return value;
        }};

    OptionBool upgrade_group_objects_upgrade{true};  // :api
    OptionPath destdir{nullptr};
    OptionString comment{nullptr};
    OptionBool downloadonly{false};  // runtime only option
    OptionBool ignorearch{false};
    OptionString module_platform_id{nullptr, ".+:.+", false};
    OptionBool module_stream_switch{false};
    OptionBool module_obsoletes{false};

    OptionString user_agent{get_user_agent()};
    OptionBool countme{false};
    OptionBool protect_running_kernel{true};
    OptionBool build_cache{true};

    // Repo main config

    OptionNumber<std::uint32_t> retries{10};
    OptionPath cachedir{geteuid() == 0 ? SYSTEM_CACHEDIR : libdnf5::xdg::get_user_cache_dir() / "libdnf5"};
    OptionBool fastestmirror{false};
    OptionStringAppendList excludepkgs{std::vector<std::string>{}};
    OptionStringAppendList includepkgs{std::vector<std::string>{}};
    OptionStringAppendList exclude_from_weak{std::vector<std::string>{}};
    OptionBool exclude_from_weak_autodetect{true};
    OptionString proxy{""};
    OptionString proxy_username{nullptr};
    OptionString proxy_password{nullptr};
    OptionStringSet proxy_auth_method{"any", "any|none|basic|digest|negotiate|ntlm|digest_ie|ntlm_wb", false};
    OptionStringAppendList protected_packages{std::vector<std::string>{"dnf5", "glob:/etc/dnf/protected.d/*.conf"}};
    OptionString username{""};
    OptionString password{""};
    OptionBool gpgcheck{false};
    OptionBool repo_gpgcheck{false};
    OptionBool enabled{true};
    OptionBool enablegroups{true};
    OptionNumber<std::uint32_t> bandwidth{0, str_to_bytes};
    OptionNumber<std::uint32_t> minrate{1000, str_to_bytes};

    OptionEnum ip_resolve{"whatever", {"ipv4", "ipv6", "whatever"}, [](const std::string & value) {
                              auto tmp = value;
                              if (value == "4") {
                                  tmp = "ipv4";
                              } else if (value == "6") {
                                  tmp = "ipv6";
                              } else {
                                  std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
                              }
                              return tmp;
                          }};

    OptionNumber<float> throttle{
        0, 0, [](const std::string & value) {
            if (!value.empty() && value.back() == '%') {
                std::size_t idx;
                auto res = std::stof(value, &idx);
                if (res < 0 || res > 100) {
                    // TODO(jrohel): Better exception info?
                    // throw Option::InvalidValue(tfm::format(_("percentage '%s' is out of range"), value));
                    throw OptionInvalidValueError(
                        M_("The throttle value {} is outside the allowed range {} ... {}"), value, 0, 100);
                }
                return res / 100;
            }
            return static_cast<float>(str_to_bytes(value));
        }};

    OptionSeconds timeout{30};
    OptionNumber<std::uint32_t> max_parallel_downloads{3, 1};
    OptionSeconds metadata_expire{60 * 60 * 48};
    OptionString sslcacert{""};
    OptionBool sslverify{true};
    OptionString sslclientcert{""};
    OptionString sslclientkey{""};
    OptionString proxy_sslcacert{""};
    OptionBool proxy_sslverify{true};
    OptionString proxy_sslclientcert{""};
    OptionString proxy_sslclientkey{""};
    OptionBool deltarpm{false};
    OptionNumber<std::uint32_t> deltarpm_percentage{75};
    OptionBool skip_if_unavailable{false};
};

ConfigMain::Impl::Impl(Config & owner) : owner(owner) {
    owner.opt_binds().add("debuglevel", debuglevel);
    owner.opt_binds().add("errorlevel", errorlevel);
    owner.opt_binds().add("installroot", installroot);
    owner.opt_binds().add("use_host_config", use_host_config);
    owner.opt_binds().add("config_file_path", config_file_path);
    owner.opt_binds().add("plugins", plugins);
    owner.opt_binds().add("pluginpath", pluginpath);
    owner.opt_binds().add("pluginconfpath", pluginconfpath);
    owner.opt_binds().add("persistdir", persistdir);

    // Unless transaction_history_dir has been explicitly set, use the system_state_dir as its default
    owner.opt_binds().add(
        "system_state_dir",
        system_state_dir,
        [&](Option::Priority priority, const std::string & value) {
            system_state_dir.set(priority, value);
            transaction_history_dir.set(Option::Priority::DEFAULT, value);
        },
        nullptr,
        false);

    owner.opt_binds().add("transaction_history_dir", transaction_history_dir);

    owner.opt_binds().add("transformdb", transformdb);
    owner.opt_binds().add("recent", recent);
    owner.opt_binds().add("reset_nice", reset_nice);
    owner.opt_binds().add("system_cachedir", system_cachedir);
    owner.opt_binds().add("cacheonly", cacheonly);
    owner.opt_binds().add("keepcache", keepcache);
    owner.opt_binds().add("logdir", logdir);
    owner.opt_binds().add("log_size", log_size);
    owner.opt_binds().add("log_rotate", log_rotate);
    owner.opt_binds().add("debugdir", debugdir);
    owner.opt_binds().add("varsdir", varsdir);
    owner.opt_binds().add("reposdir", reposdir);
    owner.opt_binds().add("debug_solver", debug_solver);
    owner.opt_binds().add("installonlypkgs", installonlypkgs);
    owner.opt_binds().add("group_package_types", group_package_types);
    owner.opt_binds().add("installonly_limit", installonly_limit);
    owner.opt_binds().add("tsflags", tsflags);
    owner.opt_binds().add("assumeyes", assumeyes);
    owner.opt_binds().add("assumeno", assumeno);
    owner.opt_binds().add("check_config_file_age", check_config_file_age);
    owner.opt_binds().add("defaultyes", defaultyes);
    owner.opt_binds().add("diskspacecheck", diskspacecheck);
    owner.opt_binds().add("localpkg_gpgcheck", localpkg_gpgcheck);
    owner.opt_binds().add("gpgkey_dns_verification", gpgkey_dns_verification);
    owner.opt_binds().add("obsoletes", obsoletes);
    owner.opt_binds().add("exit_on_lock", exit_on_lock);
    owner.opt_binds().add("allow_vendor_change", allow_vendor_change);
    owner.opt_binds().add("metadata_timer_sync", metadata_timer_sync);
    owner.opt_binds().add("disable_excludes", disable_excludes);
    owner.opt_binds().add("multilib_policy", multilib_policy);
    owner.opt_binds().add("best", best);
    owner.opt_binds().add("install_weak_deps", install_weak_deps);
    owner.opt_binds().add("allow_downgrade", allow_downgrade);
    owner.opt_binds().add("bugtracker_url", bugtracker_url);
    owner.opt_binds().add("zchunk", zchunk);
    owner.opt_binds().add("color", color);
    owner.opt_binds().add("color_list_installed_older", color_list_installed_older);
    owner.opt_binds().add("color_list_installed_newer", color_list_installed_newer);
    owner.opt_binds().add("color_list_installed_reinstall", color_list_installed_reinstall);
    owner.opt_binds().add("color_list_installed_extra", color_list_installed_extra);
    owner.opt_binds().add("color_list_available_upgrade", color_list_available_upgrade);
    owner.opt_binds().add("color_list_available_downgrade", color_list_available_downgrade);
    owner.opt_binds().add("color_list_available_reinstall", color_list_available_reinstall);
    owner.opt_binds().add("color_list_available_install", color_list_available_install);
    owner.opt_binds().add("color_update_installed", color_update_installed);
    owner.opt_binds().add("color_update_local", color_update_local);
    owner.opt_binds().add("color_update_remote", color_update_remote);
    owner.opt_binds().add("color_search_match", color_search_match);
    owner.opt_binds().add("history_record", history_record);
    owner.opt_binds().add("history_record_packages", history_record_packages);
    owner.opt_binds().add("rpmverbosity", rpmverbosity);

    // If strict value has been explicitly set, use it's negated value as a default
    // for skip_broken and skip_unavailable
    owner.opt_binds().add(
        "strict",
        strict,
        [&](Option::Priority priority, const std::string & value) {
            bool val = strict.from_string(value);
            strict.set(priority, val);
            skip_broken.set(Option::Priority::DEFAULT, !val);
            skip_unavailable.set(Option::Priority::DEFAULT, !val);
        },
        nullptr,
        false);

    owner.opt_binds().add("skip_broken", skip_broken);
    owner.opt_binds().add("skip_unavailable", skip_unavailable);
    owner.opt_binds().add("autocheck_running_kernel", autocheck_running_kernel);
    owner.opt_binds().add("clean_requirements_on_remove", clean_requirements_on_remove);
    owner.opt_binds().add("history_list_view", history_list_view);
    owner.opt_binds().add("upgrade_group_objects_upgrade", upgrade_group_objects_upgrade);
    owner.opt_binds().add("destdir", destdir);
    owner.opt_binds().add("comment", comment);
    owner.opt_binds().add("ignorearch", ignorearch);
    owner.opt_binds().add("module_platform_id", module_platform_id);
    owner.opt_binds().add("module_stream_switch", module_stream_switch);
    owner.opt_binds().add("module_obsoletes", module_obsoletes);
    owner.opt_binds().add("user_agent", user_agent);
    owner.opt_binds().add("countme", countme);
    owner.opt_binds().add("protect_running_kernel", protect_running_kernel);
    owner.opt_binds().add("build_cache", build_cache);

    // Repo main config

    owner.opt_binds().add("retries", retries);
    owner.opt_binds().add("cachedir", cachedir);
    owner.opt_binds().add("fastestmirror", fastestmirror);
    owner.opt_binds().add("excludepkgs", excludepkgs);
    owner.opt_binds().add("exclude", excludepkgs);  //compatibility with yum
    owner.opt_binds().add("includepkgs", includepkgs);
    owner.opt_binds().add("exclude_from_weak", exclude_from_weak);
    owner.opt_binds().add("exclude_from_weak_autodetect", exclude_from_weak_autodetect);
    owner.opt_binds().add("proxy", proxy);
    owner.opt_binds().add("proxy_username", proxy_username);
    owner.opt_binds().add("proxy_password", proxy_password);

    owner.opt_binds().add(
        "proxy_auth_method",
        proxy_auth_method,
        [&](Option::Priority priority, const std::string & value) {
            if (priority >= proxy_auth_method.get_priority()) {
                auto tmp = value;
                std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
                proxy_auth_method.set(priority, tmp);
            }
        },
        nullptr,
        false);

    owner.opt_binds().add("protected_packages", protected_packages);
    owner.opt_binds().add("username", username);
    owner.opt_binds().add("password", password);
    owner.opt_binds().add("gpgcheck", gpgcheck);
    owner.opt_binds().add("repo_gpgcheck", repo_gpgcheck);
    owner.opt_binds().add("enabled", enabled);
    owner.opt_binds().add("enablegroups", enablegroups);
    owner.opt_binds().add("bandwidth", bandwidth);
    owner.opt_binds().add("minrate", minrate);
    owner.opt_binds().add("ip_resolve", ip_resolve);
    owner.opt_binds().add("throttle", throttle);
    owner.opt_binds().add("timeout", timeout);
    owner.opt_binds().add("max_parallel_downloads", max_parallel_downloads);
    owner.opt_binds().add("metadata_expire", metadata_expire);
    owner.opt_binds().add("sslcacert", sslcacert);
    owner.opt_binds().add("sslverify", sslverify);
    owner.opt_binds().add("sslclientcert", sslclientcert);
    owner.opt_binds().add("sslclientkey", sslclientkey);
    owner.opt_binds().add("proxy_sslcacert", proxy_sslcacert);
    owner.opt_binds().add("proxy_sslverify", proxy_sslverify);
    owner.opt_binds().add("proxy_sslclientcert", proxy_sslclientcert);
    owner.opt_binds().add("proxy_sslclientkey", proxy_sslclientkey);
    owner.opt_binds().add("deltarpm", deltarpm);
    owner.opt_binds().add("deltarpm_percentage", deltarpm_percentage);
    owner.opt_binds().add("skip_if_unavailable", skip_if_unavailable);
    owner.opt_binds().add("optional_metadata_types", optional_metadata_types);
}

ConfigMain::ConfigMain() {
    p_impl = std::unique_ptr<Impl>(new Impl(*this));
}
ConfigMain::~ConfigMain() = default;

OptionNumber<std::int32_t> & ConfigMain::get_debuglevel_option() {
    return p_impl->debuglevel;
}
const OptionNumber<std::int32_t> & ConfigMain::get_debuglevel_option() const {
    return p_impl->debuglevel;
}

OptionNumber<std::int32_t> & ConfigMain::get_errorlevel_option() {
    return p_impl->errorlevel;
}
const OptionNumber<std::int32_t> & ConfigMain::get_errorlevel_option() const {
    return p_impl->errorlevel;
}

OptionPath & ConfigMain::get_installroot_option() {
    return p_impl->installroot;
}
const OptionPath & ConfigMain::get_installroot_option() const {
    return p_impl->installroot;
}

OptionBool & ConfigMain::get_use_host_config_option() {
    return p_impl->use_host_config;
}
const OptionBool & ConfigMain::get_use_host_config_option() const {
    return p_impl->use_host_config;
}

OptionPath & ConfigMain::get_config_file_path_option() {
    return p_impl->config_file_path;
}
const OptionPath & ConfigMain::get_config_file_path_option() const {
    return p_impl->config_file_path;
}

OptionBool & ConfigMain::get_plugins_option() {
    return p_impl->plugins;
}
const OptionBool & ConfigMain::get_plugins_option() const {
    return p_impl->plugins;
}

OptionPath & ConfigMain::get_pluginpath_option() {
    return p_impl->pluginpath;
}
const OptionPath & ConfigMain::get_pluginpath_option() const {
    return p_impl->pluginpath;
}

OptionPath & ConfigMain::get_pluginconfpath_option() {
    return p_impl->pluginconfpath;
}
const OptionPath & ConfigMain::get_pluginconfpath_option() const {
    return p_impl->pluginconfpath;
}

OptionPath & ConfigMain::get_persistdir_option() {
    return p_impl->persistdir;
}
const OptionPath & ConfigMain::get_persistdir_option() const {
    return p_impl->persistdir;
}

OptionPath & ConfigMain::get_system_state_dir_option() {
    return p_impl->system_state_dir;
}

const OptionPath & ConfigMain::get_system_state_dir_option() const {
    return p_impl->system_state_dir;
}

OptionPath & ConfigMain::get_transaction_history_dir_option() {
    return p_impl->transaction_history_dir;
}

const OptionPath & ConfigMain::get_transaction_history_dir_option() const {
    return p_impl->transaction_history_dir;
}


OptionBool & ConfigMain::get_transformdb_option() {
    return p_impl->transformdb;
}
const OptionBool & ConfigMain::get_transformdb_option() const {
    return p_impl->transformdb;
}

OptionNumber<std::int32_t> & ConfigMain::get_recent_option() {
    return p_impl->recent;
}
const OptionNumber<std::int32_t> & ConfigMain::get_recent_option() const {
    return p_impl->recent;
}

OptionBool & ConfigMain::get_reset_nice_option() {
    return p_impl->reset_nice;
}
const OptionBool & ConfigMain::get_reset_nice_option() const {
    return p_impl->reset_nice;
}

OptionPath & ConfigMain::get_system_cachedir_option() {
    return p_impl->system_cachedir;
}
const OptionPath & ConfigMain::get_system_cachedir_option() const {
    return p_impl->system_cachedir;
}

OptionEnum & ConfigMain::get_cacheonly_option() {
    return p_impl->cacheonly;
}
const OptionEnum & ConfigMain::get_cacheonly_option() const {
    return p_impl->cacheonly;
}

OptionBool & ConfigMain::get_keepcache_option() {
    return p_impl->keepcache;
}
const OptionBool & ConfigMain::get_keepcache_option() const {
    return p_impl->keepcache;
}

OptionPath & ConfigMain::get_logdir_option() {
    return p_impl->logdir;
}
const OptionPath & ConfigMain::get_logdir_option() const {
    return p_impl->logdir;
}

OptionNumber<std::int32_t> & ConfigMain::get_log_size_option() {
    return p_impl->log_size;
}
const OptionNumber<std::int32_t> & ConfigMain::get_log_size_option() const {
    return p_impl->log_size;
}

OptionNumber<std::int32_t> & ConfigMain::get_log_rotate_option() {
    return p_impl->log_rotate;
}
const OptionNumber<std::int32_t> & ConfigMain::get_log_rotate_option() const {
    return p_impl->log_rotate;
}

OptionPath & ConfigMain::get_debugdir_option() {
    return p_impl->debugdir;
}
const OptionPath & ConfigMain::get_debugdir_option() const {
    return p_impl->debugdir;
}

OptionStringList & ConfigMain::get_varsdir_option() {
    return p_impl->varsdir;
}
const OptionStringList & ConfigMain::get_varsdir_option() const {
    return p_impl->varsdir;
}

OptionStringList & ConfigMain::get_reposdir_option() {
    return p_impl->reposdir;
}
const OptionStringList & ConfigMain::get_reposdir_option() const {
    return p_impl->reposdir;
}

OptionBool & ConfigMain::get_debug_solver_option() {
    return p_impl->debug_solver;
}
const OptionBool & ConfigMain::get_debug_solver_option() const {
    return p_impl->debug_solver;
}

OptionStringAppendList & ConfigMain::get_installonlypkgs_option() {
    return p_impl->installonlypkgs;
}
const OptionStringAppendList & ConfigMain::get_installonlypkgs_option() const {
    return p_impl->installonlypkgs;
}

OptionStringList & ConfigMain::get_group_package_types_option() {
    return p_impl->group_package_types;
}
const OptionStringList & ConfigMain::get_group_package_types_option() const {
    return p_impl->group_package_types;
}

OptionStringAppendSet & ConfigMain::get_optional_metadata_types_option() {
    return p_impl->optional_metadata_types;
}
const OptionStringAppendSet & ConfigMain::get_optional_metadata_types_option() const {
    return p_impl->optional_metadata_types;
}

OptionNumber<std::uint32_t> & ConfigMain::get_installonly_limit_option() {
    return p_impl->installonly_limit;
}
const OptionNumber<std::uint32_t> & ConfigMain::get_installonly_limit_option() const {
    return p_impl->installonly_limit;
}

OptionStringAppendList & ConfigMain::get_tsflags_option() {
    return p_impl->tsflags;
}
const OptionStringAppendList & ConfigMain::get_tsflags_option() const {
    return p_impl->tsflags;
}

OptionBool & ConfigMain::get_assumeyes_option() {
    return p_impl->assumeyes;
}
const OptionBool & ConfigMain::get_assumeyes_option() const {
    return p_impl->assumeyes;
}

OptionBool & ConfigMain::get_assumeno_option() {
    return p_impl->assumeno;
}
const OptionBool & ConfigMain::get_assumeno_option() const {
    return p_impl->assumeno;
}

OptionBool & ConfigMain::get_check_config_file_age_option() {
    return p_impl->check_config_file_age;
}
const OptionBool & ConfigMain::get_check_config_file_age_option() const {
    return p_impl->check_config_file_age;
}

OptionBool & ConfigMain::get_defaultyes_option() {
    return p_impl->defaultyes;
}
const OptionBool & ConfigMain::get_defaultyes_option() const {
    return p_impl->defaultyes;
}

OptionBool & ConfigMain::get_diskspacecheck_option() {
    return p_impl->diskspacecheck;
}
const OptionBool & ConfigMain::get_diskspacecheck_option() const {
    return p_impl->diskspacecheck;
}

OptionBool & ConfigMain::get_localpkg_gpgcheck_option() {
    return p_impl->localpkg_gpgcheck;
}
const OptionBool & ConfigMain::get_localpkg_gpgcheck_option() const {
    return p_impl->localpkg_gpgcheck;
}

OptionBool & ConfigMain::get_gpgkey_dns_verification_option() {
    return p_impl->gpgkey_dns_verification;
}
const OptionBool & ConfigMain::get_gpgkey_dns_verification_option() const {
    return p_impl->gpgkey_dns_verification;
}

OptionBool & ConfigMain::get_obsoletes_option() {
    return p_impl->obsoletes;
}
const OptionBool & ConfigMain::get_obsoletes_option() const {
    return p_impl->obsoletes;
}

OptionBool & ConfigMain::get_exit_on_lock_option() {
    return p_impl->exit_on_lock;
}
const OptionBool & ConfigMain::get_exit_on_lock_option() const {
    return p_impl->exit_on_lock;
}

OptionBool & ConfigMain::get_allow_vendor_change_option() {
    return p_impl->allow_vendor_change;
}
const OptionBool & ConfigMain::get_allow_vendor_change_option() const {
    return p_impl->allow_vendor_change;
}

OptionSeconds & ConfigMain::get_metadata_timer_sync_option() {
    return p_impl->metadata_timer_sync;
}
const OptionSeconds & ConfigMain::get_metadata_timer_sync_option() const {
    return p_impl->metadata_timer_sync;
}

OptionStringList & ConfigMain::get_disable_excludes_option() {
    return p_impl->disable_excludes;
}
const OptionStringList & ConfigMain::get_disable_excludes_option() const {
    return p_impl->disable_excludes;
}

OptionEnum & ConfigMain::get_multilib_policy_option() {
    return p_impl->multilib_policy;
}
const OptionEnum & ConfigMain::get_multilib_policy_option() const {
    return p_impl->multilib_policy;
}

OptionBool & ConfigMain::get_best_option() {
    return p_impl->best;
}
const OptionBool & ConfigMain::get_best_option() const {
    return p_impl->best;
}

OptionBool & ConfigMain::get_install_weak_deps_option() {
    return p_impl->install_weak_deps;
}
const OptionBool & ConfigMain::get_install_weak_deps_option() const {
    return p_impl->install_weak_deps;
}

OptionBool & ConfigMain::get_allow_downgrade_option() {
    return p_impl->allow_downgrade;
}
const OptionBool & ConfigMain::get_allow_downgrade_option() const {
    return p_impl->allow_downgrade;
}

OptionString & ConfigMain::get_bugtracker_url_option() {
    return p_impl->bugtracker_url;
}
const OptionString & ConfigMain::get_bugtracker_url_option() const {
    return p_impl->bugtracker_url;
}

OptionBool & ConfigMain::get_zchunk_option() {
    return p_impl->zchunk;
}
const OptionBool & ConfigMain::get_zchunk_option() const {
    return p_impl->zchunk;
}

OptionEnum & ConfigMain::get_color_option() {
    return p_impl->color;
}
const OptionEnum & ConfigMain::get_color_option() const {
    return p_impl->color;
}

OptionString & ConfigMain::get_color_list_installed_older_option() {
    return p_impl->color_list_installed_older;
}
const OptionString & ConfigMain::get_color_list_installed_older_option() const {
    return p_impl->color_list_installed_older;
}

OptionString & ConfigMain::get_color_list_installed_newer_option() {
    return p_impl->color_list_installed_newer;
}
const OptionString & ConfigMain::get_color_list_installed_newer_option() const {
    return p_impl->color_list_installed_newer;
}

OptionString & ConfigMain::get_color_list_installed_reinstall_option() {
    return p_impl->color_list_installed_reinstall;
}
const OptionString & ConfigMain::get_color_list_installed_reinstall_option() const {
    return p_impl->color_list_installed_reinstall;
}

OptionString & ConfigMain::get_color_list_installed_extra_option() {
    return p_impl->color_list_installed_extra;
}
const OptionString & ConfigMain::get_color_list_installed_extra_option() const {
    return p_impl->color_list_installed_extra;
}

OptionString & ConfigMain::get_color_list_available_upgrade_option() {
    return p_impl->color_list_available_upgrade;
}
const OptionString & ConfigMain::get_color_list_available_upgrade_option() const {
    return p_impl->color_list_available_upgrade;
}

OptionString & ConfigMain::get_color_list_available_downgrade_option() {
    return p_impl->color_list_available_downgrade;
}
const OptionString & ConfigMain::get_color_list_available_downgrade_option() const {
    return p_impl->color_list_available_downgrade;
}

OptionString & ConfigMain::get_color_list_available_reinstall_option() {
    return p_impl->color_list_available_reinstall;
}
const OptionString & ConfigMain::get_color_list_available_reinstall_option() const {
    return p_impl->color_list_available_reinstall;
}

OptionString & ConfigMain::get_color_list_available_install_option() {
    return p_impl->color_list_available_install;
}
const OptionString & ConfigMain::get_color_list_available_install_option() const {
    return p_impl->color_list_available_install;
}

OptionString & ConfigMain::get_color_update_installed_option() {
    return p_impl->color_update_installed;
}
const OptionString & ConfigMain::get_color_update_installed_option() const {
    return p_impl->color_update_installed;
}

OptionString & ConfigMain::get_color_update_local_option() {
    return p_impl->color_update_local;
}
const OptionString & ConfigMain::get_color_update_local_option() const {
    return p_impl->color_update_local;
}

OptionString & ConfigMain::get_color_update_remote_option() {
    return p_impl->color_update_remote;
}
const OptionString & ConfigMain::get_color_update_remote_option() const {
    return p_impl->color_update_remote;
}

OptionString & ConfigMain::get_color_search_match_option() {
    return p_impl->color_search_match;
}
const OptionString & ConfigMain::get_color_search_match_option() const {
    return p_impl->color_search_match;
}

OptionBool & ConfigMain::get_history_record_option() {
    return p_impl->history_record;
}
const OptionBool & ConfigMain::get_history_record_option() const {
    return p_impl->history_record;
}

OptionStringList & ConfigMain::get_history_record_packages_option() {
    return p_impl->history_record_packages;
}
const OptionStringList & ConfigMain::get_history_record_packages_option() const {
    return p_impl->history_record_packages;
}

OptionString & ConfigMain::get_rpmverbosity_option() {
    return p_impl->rpmverbosity;
}
const OptionString & ConfigMain::get_rpmverbosity_option() const {
    return p_impl->rpmverbosity;
}

OptionBool & ConfigMain::get_strict_option() {
    return p_impl->strict;
}
const OptionBool & ConfigMain::get_strict_option() const {
    return p_impl->strict;
}

OptionBool & ConfigMain::get_skip_broken_option() {
    return p_impl->skip_broken;
}
const OptionBool & ConfigMain::get_skip_broken_option() const {
    return p_impl->skip_broken;
}

OptionBool & ConfigMain::get_skip_unavailable_option() {
    return p_impl->skip_unavailable;
}
const OptionBool & ConfigMain::get_skip_unavailable_option() const {
    return p_impl->skip_unavailable;
}

OptionBool & ConfigMain::get_autocheck_running_kernel_option() {
    return p_impl->autocheck_running_kernel;
}
const OptionBool & ConfigMain::get_autocheck_running_kernel_option() const {
    return p_impl->autocheck_running_kernel;
}

OptionBool & ConfigMain::get_clean_requirements_on_remove_option() {
    return p_impl->clean_requirements_on_remove;
}
const OptionBool & ConfigMain::get_clean_requirements_on_remove_option() const {
    return p_impl->clean_requirements_on_remove;
}

OptionEnum & ConfigMain::get_history_list_view_option() {
    return p_impl->history_list_view;
}
const OptionEnum & ConfigMain::get_history_list_view_option() const {
    return p_impl->history_list_view;
}

OptionBool & ConfigMain::get_upgrade_group_objects_upgrade_option() {
    return p_impl->upgrade_group_objects_upgrade;
}
const OptionBool & ConfigMain::get_upgrade_group_objects_upgrade_option() const {
    return p_impl->upgrade_group_objects_upgrade;
}

OptionPath & ConfigMain::get_destdir_option() {
    return p_impl->destdir;
}
const OptionPath & ConfigMain::get_destdir_option() const {
    return p_impl->destdir;
}

OptionString & ConfigMain::get_comment_option() {
    return p_impl->comment;
}
const OptionString & ConfigMain::get_comment_option() const {
    return p_impl->comment;
}

OptionBool & ConfigMain::get_downloadonly_option() {
    return p_impl->downloadonly;
}
const OptionBool & ConfigMain::get_downloadonly_option() const {
    return p_impl->downloadonly;
}

OptionBool & ConfigMain::get_ignorearch_option() {
    return p_impl->ignorearch;
}
const OptionBool & ConfigMain::get_ignorearch_option() const {
    return p_impl->ignorearch;
}

OptionString & ConfigMain::get_module_platform_id_option() {
    return p_impl->module_platform_id;
}
const OptionString & ConfigMain::get_module_platform_id_option() const {
    return p_impl->module_platform_id;
}

OptionBool & ConfigMain::get_module_stream_switch_option() {
    return p_impl->module_stream_switch;
}

const OptionBool & ConfigMain::get_module_stream_switch_option() const {
    return p_impl->module_stream_switch;
}

OptionBool & ConfigMain::get_module_obsoletes_option() {
    return p_impl->module_obsoletes;
}

const OptionBool & ConfigMain::get_module_obsoletes_option() const {
    return p_impl->module_obsoletes;
}

OptionString & ConfigMain::get_user_agent_option() {
    return p_impl->user_agent;
}
const OptionString & ConfigMain::get_user_agent_option() const {
    return p_impl->user_agent;
}

OptionBool & ConfigMain::get_countme_option() {
    return p_impl->countme;
}
const OptionBool & ConfigMain::get_countme_option() const {
    return p_impl->countme;
}

OptionBool & ConfigMain::get_protect_running_kernel_option() {
    return p_impl->protect_running_kernel;
}

const OptionBool & ConfigMain::get_protect_running_kernel_option() const {
    return p_impl->protect_running_kernel;
}

OptionBool & ConfigMain::get_build_cache_option() {
    return p_impl->build_cache;
}
const OptionBool & ConfigMain::get_build_cache_option() const {
    return p_impl->build_cache;
}

// Repo main config
OptionNumber<std::uint32_t> & ConfigMain::get_retries_option() {
    return p_impl->retries;
}
const OptionNumber<std::uint32_t> & ConfigMain::get_retries_option() const {
    return p_impl->retries;
}

OptionPath & ConfigMain::get_cachedir_option() {
    return p_impl->cachedir;
}
const OptionPath & ConfigMain::get_cachedir_option() const {
    return p_impl->cachedir;
}

OptionBool & ConfigMain::get_fastestmirror_option() {
    return p_impl->fastestmirror;
}
const OptionBool & ConfigMain::get_fastestmirror_option() const {
    return p_impl->fastestmirror;
}

OptionStringAppendList & ConfigMain::get_excludepkgs_option() {
    return p_impl->excludepkgs;
}
const OptionStringAppendList & ConfigMain::get_excludepkgs_option() const {
    return p_impl->excludepkgs;
}

OptionStringAppendList & ConfigMain::get_includepkgs_option() {
    return p_impl->includepkgs;
}
const OptionStringAppendList & ConfigMain::get_includepkgs_option() const {
    return p_impl->includepkgs;
}

OptionStringAppendList & ConfigMain::get_exclude_from_weak_option() {
    return p_impl->exclude_from_weak;
}

const OptionStringAppendList & ConfigMain::get_exclude_from_weak_option() const {
    return p_impl->exclude_from_weak;
}

OptionBool & ConfigMain::get_exclude_from_weak_autodetect_option() {
    return p_impl->exclude_from_weak_autodetect;
}

const OptionBool & ConfigMain::get_exclude_from_weak_autodetect_option() const {
    return p_impl->exclude_from_weak_autodetect;
}

OptionString & ConfigMain::get_proxy_option() {
    return p_impl->proxy;
}
const OptionString & ConfigMain::get_proxy_option() const {
    return p_impl->proxy;
}

OptionString & ConfigMain::get_proxy_username_option() {
    return p_impl->proxy_username;
}
const OptionString & ConfigMain::get_proxy_username_option() const {
    return p_impl->proxy_username;
}

OptionString & ConfigMain::get_proxy_password_option() {
    return p_impl->proxy_password;
}
const OptionString & ConfigMain::get_proxy_password_option() const {
    return p_impl->proxy_password;
}

OptionStringSet & ConfigMain::get_proxy_auth_method_option() {
    return p_impl->proxy_auth_method;
}
const OptionStringSet & ConfigMain::get_proxy_auth_method_option() const {
    return p_impl->proxy_auth_method;
}

OptionStringAppendList & ConfigMain::get_protected_packages_option() {
    return p_impl->protected_packages;
}
const OptionStringAppendList & ConfigMain::get_protected_packages_option() const {
    return p_impl->protected_packages;
}

OptionString & ConfigMain::get_username_option() {
    return p_impl->username;
}
const OptionString & ConfigMain::get_username_option() const {
    return p_impl->username;
}

OptionString & ConfigMain::get_password_option() {
    return p_impl->password;
}
const OptionString & ConfigMain::get_password_option() const {
    return p_impl->password;
}

OptionBool & ConfigMain::get_gpgcheck_option() {
    return p_impl->gpgcheck;
}
const OptionBool & ConfigMain::get_gpgcheck_option() const {
    return p_impl->gpgcheck;
}

OptionBool & ConfigMain::get_repo_gpgcheck_option() {
    return p_impl->repo_gpgcheck;
}
const OptionBool & ConfigMain::get_repo_gpgcheck_option() const {
    return p_impl->repo_gpgcheck;
}

OptionBool & ConfigMain::get_enabled_option() {
    return p_impl->enabled;
}
const OptionBool & ConfigMain::get_enabled_option() const {
    return p_impl->enabled;
}

OptionBool & ConfigMain::get_enablegroups_option() {
    return p_impl->enablegroups;
}
const OptionBool & ConfigMain::get_enablegroups_option() const {
    return p_impl->enablegroups;
}

OptionNumber<std::uint32_t> & ConfigMain::get_bandwidth_option() {
    return p_impl->bandwidth;
}
const OptionNumber<std::uint32_t> & ConfigMain::get_bandwidth_option() const {
    return p_impl->bandwidth;
}

OptionNumber<std::uint32_t> & ConfigMain::get_minrate_option() {
    return p_impl->minrate;
}
const OptionNumber<std::uint32_t> & ConfigMain::get_minrate_option() const {
    return p_impl->minrate;
}

OptionEnum & ConfigMain::get_ip_resolve_option() {
    return p_impl->ip_resolve;
}
const OptionEnum & ConfigMain::get_ip_resolve_option() const {
    return p_impl->ip_resolve;
}

OptionNumber<float> & ConfigMain::get_throttle_option() {
    return p_impl->throttle;
}
const OptionNumber<float> & ConfigMain::get_throttle_option() const {
    return p_impl->throttle;
}

OptionSeconds & ConfigMain::get_timeout_option() {
    return p_impl->timeout;
}
const OptionSeconds & ConfigMain::get_timeout_option() const {
    return p_impl->timeout;
}

OptionNumber<std::uint32_t> & ConfigMain::get_max_parallel_downloads_option() {
    return p_impl->max_parallel_downloads;
}
const OptionNumber<std::uint32_t> & ConfigMain::get_max_parallel_downloads_option() const {
    return p_impl->max_parallel_downloads;
}

OptionSeconds & ConfigMain::get_metadata_expire_option() {
    return p_impl->metadata_expire;
}
const OptionSeconds & ConfigMain::get_metadata_expire_option() const {
    return p_impl->metadata_expire;
}

OptionString & ConfigMain::get_sslcacert_option() {
    return p_impl->sslcacert;
}
const OptionString & ConfigMain::get_sslcacert_option() const {
    return p_impl->sslcacert;
}

OptionBool & ConfigMain::get_sslverify_option() {
    return p_impl->sslverify;
}
const OptionBool & ConfigMain::get_sslverify_option() const {
    return p_impl->sslverify;
}

OptionString & ConfigMain::get_sslclientcert_option() {
    return p_impl->sslclientcert;
}
const OptionString & ConfigMain::get_sslclientcert_option() const {
    return p_impl->sslclientcert;
}

OptionString & ConfigMain::get_sslclientkey_option() {
    return p_impl->sslclientkey;
}
const OptionString & ConfigMain::get_sslclientkey_option() const {
    return p_impl->sslclientkey;
}

OptionString & ConfigMain::get_proxy_sslcacert_option() {
    return p_impl->proxy_sslcacert;
}

const OptionString & ConfigMain::get_proxy_sslcacert_option() const {
    return p_impl->proxy_sslcacert;
}

OptionBool & ConfigMain::get_proxy_sslverify_option() {
    return p_impl->proxy_sslverify;
}

const OptionBool & ConfigMain::get_proxy_sslverify_option() const {
    return p_impl->proxy_sslverify;
}

OptionString & ConfigMain::get_proxy_sslclientcert_option() {
    return p_impl->proxy_sslclientcert;
}

const OptionString & ConfigMain::get_proxy_sslclientcert_option() const {
    return p_impl->proxy_sslclientcert;
}

OptionString & ConfigMain::get_proxy_sslclientkey_option() {
    return p_impl->proxy_sslclientkey;
}

const OptionString & ConfigMain::get_proxy_sslclientkey_option() const {
    return p_impl->proxy_sslclientkey;
}

OptionBool & ConfigMain::get_deltarpm_option() {
    return p_impl->deltarpm;
}
const OptionBool & ConfigMain::get_deltarpm_option() const {
    return p_impl->deltarpm;
}

OptionNumber<std::uint32_t> & ConfigMain::get_deltarpm_percentage_option() {
    return p_impl->deltarpm_percentage;
}
const OptionNumber<std::uint32_t> & ConfigMain::get_deltarpm_percentage_option() const {
    return p_impl->deltarpm_percentage;
}

OptionBool & ConfigMain::get_skip_if_unavailable_option() {
    return p_impl->skip_if_unavailable;
}
const OptionBool & ConfigMain::get_skip_if_unavailable_option() const {
    return p_impl->skip_if_unavailable;
}

void ConfigMain::load_from_parser(
    const ConfigParser & parser,
    const std::string & section,
    const Vars & vars,
    Logger & logger,
    Option::Priority priority) {
    Config::load_from_parser(parser, section, vars, logger, priority);

    if (geteuid() == 0) {
        p_impl->cachedir.set(Option::Priority::MAINCONFIG, p_impl->system_cachedir.get_value());
    }
}

}  // namespace libdnf5
