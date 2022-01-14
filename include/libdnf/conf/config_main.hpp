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

#ifndef LIBDNF_CONF_CONFIG_MAIN_HPP
#define LIBDNF_CONF_CONFIG_MAIN_HPP

#include "config.hpp"
#include "option_bool.hpp"
#include "option_enum.hpp"
#include "option_number.hpp"
#include "option_path.hpp"
#include "option_seconds.hpp"
#include "option_string.hpp"
#include "option_string_list.hpp"

#include "libdnf/logger/logger.hpp"

#include <memory>


namespace libdnf {

/// Holds global configuration
class ConfigMain : public Config<Option::Priority::MAINCONFIG> {
public:
    ConfigMain();
    ~ConfigMain();

    OptionNumber<std::int32_t> & debuglevel();
    const OptionNumber<std::int32_t> & debuglevel() const;
    OptionNumber<std::int32_t> & errorlevel();
    const OptionNumber<std::int32_t> & errorlevel() const;
    OptionString & installroot();
    const OptionString & installroot() const;
    OptionString & config_file_path();
    const OptionString & config_file_path() const;
    OptionBool & plugins();
    const OptionBool & plugins() const;
    OptionStringList & pluginpath();
    const OptionStringList & pluginpath() const;
    OptionStringList & pluginconfpath();
    const OptionStringList & pluginconfpath() const;
    OptionString & persistdir();
    const OptionString & persistdir() const;
    OptionBool & transformdb();
    const OptionBool & transformdb() const;
    OptionNumber<std::int32_t> & recent();
    const OptionNumber<std::int32_t> & recent() const;
    OptionBool & reset_nice();
    const OptionBool & reset_nice() const;
    OptionString & system_cachedir();
    const OptionString & system_cachedir() const;
    OptionBool & cacheonly();
    const OptionBool & cacheonly() const;
    OptionBool & keepcache();
    const OptionBool & keepcache() const;
    OptionString & logdir();
    const OptionString & logdir() const;
    OptionNumber<std::int32_t> & log_size();
    const OptionNumber<std::int32_t> & log_size() const;
    OptionNumber<std::int32_t> & log_rotate();
    const OptionNumber<std::int32_t> & log_rotate() const;
    OptionStringList & varsdir();
    const OptionStringList & varsdir() const;
    OptionStringList & reposdir();
    const OptionStringList & reposdir() const;
    OptionBool & debug_solver();
    const OptionBool & debug_solver() const;
    OptionStringList & installonlypkgs();
    const OptionStringList & installonlypkgs() const;
    OptionStringList & group_package_types();
    const OptionStringList & group_package_types() const;

    //  NOTE: If you set this to 2, then because it keeps the current
    // kernel it means if you ever install an "old" kernel it'll get rid
    // of the newest one so you probably want to use 3 as a minimum
    // ... if you turn it on.
    OptionNumber<std::uint32_t> & installonly_limit();
    const OptionNumber<std::uint32_t> & installonly_limit() const;

    OptionStringList & tsflags();
    const OptionStringList & tsflags() const;
    OptionBool & assumeyes();
    const OptionBool & assumeyes() const;
    OptionBool & assumeno();
    const OptionBool & assumeno() const;
    OptionBool & check_config_file_age();
    const OptionBool & check_config_file_age() const;
    OptionBool & defaultyes();
    const OptionBool & defaultyes() const;
    OptionBool & diskspacecheck();
    const OptionBool & diskspacecheck() const;
    OptionBool & localpkg_gpgcheck();
    const OptionBool & localpkg_gpgcheck() const;
    OptionBool & gpgkey_dns_verification();
    const OptionBool & gpgkey_dns_verification() const;
    OptionBool & obsoletes();
    const OptionBool & obsoletes() const;
    OptionBool & showdupesfromrepos();
    const OptionBool & showdupesfromrepos() const;
    OptionBool & exit_on_lock();
    const OptionBool & exit_on_lock() const;
    OptionBool & allow_vendor_change();
    const OptionBool & allow_vendor_change() const;
    OptionSeconds & metadata_timer_sync();
    const OptionSeconds & metadata_timer_sync() const;
    OptionStringList & disable_excludes();
    const OptionStringList & disable_excludes() const;
    OptionEnum<std::string> & multilib_policy();  // :api
    const OptionEnum<std::string> & multilib_policy() const;
    OptionBool & best();  // :api
    const OptionBool & best() const;
    OptionBool & install_weak_deps();
    const OptionBool & install_weak_deps() const;
    OptionString & bugtracker_url();
    const OptionString & bugtracker_url() const;
    OptionBool & zchunk();
    const OptionBool & zchunk() const;
    OptionEnum<std::string> & color();
    const OptionEnum<std::string> & color() const;
    OptionString & color_list_installed_older();
    const OptionString & color_list_installed_older() const;
    OptionString & color_list_installed_newer();
    const OptionString & color_list_installed_newer() const;
    OptionString & color_list_installed_reinstall();
    const OptionString & color_list_installed_reinstall() const;
    OptionString & color_list_installed_extra();
    const OptionString & color_list_installed_extra() const;
    OptionString & color_list_available_upgrade();
    const OptionString & color_list_available_upgrade() const;
    OptionString & color_list_available_downgrade();
    const OptionString & color_list_available_downgrade() const;
    OptionString & color_list_available_reinstall();
    const OptionString & color_list_available_reinstall() const;
    OptionString & color_list_available_install();
    const OptionString & color_list_available_install() const;
    OptionString & color_update_installed();
    const OptionString & color_update_installed() const;
    OptionString & color_update_local();
    const OptionString & color_update_local() const;
    OptionString & color_update_remote();
    const OptionString & color_update_remote() const;
    OptionString & color_search_match();
    const OptionString & color_search_match() const;
    OptionBool & history_record();
    const OptionBool & history_record() const;
    OptionStringList & history_record_packages();
    const OptionStringList & history_record_packages() const;
    OptionString & rpmverbosity();
    const OptionString & rpmverbosity() const;
    OptionBool & strict();  // :api
    const OptionBool & strict() const;
    OptionBool & skip_broken();  // :yum-compatibility
    const OptionBool & skip_broken() const;
    OptionBool & autocheck_running_kernel();  // :yum-compatibility
    const OptionBool & autocheck_running_kernel() const;
    OptionBool & clean_requirements_on_remove();
    const OptionBool & clean_requirements_on_remove() const;
    OptionEnum<std::string> & history_list_view();
    const OptionEnum<std::string> & history_list_view() const;
    OptionBool & upgrade_group_objects_upgrade();
    const OptionBool & upgrade_group_objects_upgrade() const;
    OptionPath & destdir();
    const OptionPath & destdir() const;
    OptionString & comment();
    const OptionString & comment() const;
    OptionBool & downloadonly();
    const OptionBool & downloadonly() const;
    OptionBool & ignorearch();
    const OptionBool & ignorearch() const;

    OptionString & module_platform_id();
    const OptionString & module_platform_id() const;
    OptionBool & module_stream_switch();
    const OptionBool & module_stream_switch() const;
    OptionBool & module_obsoletes();
    const OptionBool & module_obsoletes() const;
    OptionString & user_agent();
    const OptionString & user_agent() const;
    OptionBool & countme();
    const OptionBool & countme() const;
    OptionBool & protect_running_kernel();
    const OptionBool & protect_running_kernel() const;
    OptionBool & build_cache();
    const OptionBool & build_cache() const;

    // Repo main config
    OptionNumber<std::uint32_t> & retries();
    const OptionNumber<std::uint32_t> & retries() const;
    OptionString & cachedir();
    const OptionString & cachedir() const;
    OptionBool & fastestmirror();
    const OptionBool & fastestmirror() const;
    OptionStringList & excludepkgs();
    const OptionStringList & excludepkgs() const;
    OptionStringList & includepkgs();
    const OptionStringList & includepkgs() const;
    OptionString & proxy();
    const OptionString & proxy() const;
    OptionString & proxy_username();
    const OptionString & proxy_username() const;
    OptionString & proxy_password();
    const OptionString & proxy_password() const;
    OptionEnum<std::string> & proxy_auth_method();
    const OptionEnum<std::string> & proxy_auth_method() const;
    OptionStringList & protected_packages();
    const OptionStringList & protected_packages() const;
    OptionString & username();
    const OptionString & username() const;
    OptionString & password();
    const OptionString & password() const;
    OptionBool & gpgcheck();
    const OptionBool & gpgcheck() const;
    OptionBool & repo_gpgcheck();
    const OptionBool & repo_gpgcheck() const;
    OptionBool & enabled();
    const OptionBool & enabled() const;
    OptionBool & enablegroups();
    const OptionBool & enablegroups() const;
    OptionNumber<std::uint32_t> & bandwidth();
    const OptionNumber<std::uint32_t> & bandwidth() const;
    OptionNumber<std::uint32_t> & minrate();
    const OptionNumber<std::uint32_t> & minrate() const;
    OptionEnum<std::string> & ip_resolve();
    const OptionEnum<std::string> & ip_resolve() const;
    OptionNumber<float> & throttle();
    const OptionNumber<float> & throttle() const;
    OptionSeconds & timeout();
    const OptionSeconds & timeout() const;
    OptionNumber<std::uint32_t> & max_parallel_downloads();
    const OptionNumber<std::uint32_t> & max_parallel_downloads() const;
    OptionSeconds & metadata_expire();
    const OptionSeconds & metadata_expire() const;
    OptionString & sslcacert();
    const OptionString & sslcacert() const;
    OptionBool & sslverify();
    const OptionBool & sslverify() const;
    OptionString & sslclientcert();
    const OptionString & sslclientcert() const;
    OptionString & sslclientkey();
    const OptionString & sslclientkey() const;
    OptionString & proxy_sslcacert();
    const OptionString & proxy_sslcacert() const;
    OptionBool & proxy_sslverify();
    const OptionBool & proxy_sslverify() const;
    OptionString & proxy_sslclientcert();
    const OptionString & proxy_sslclientcert() const;
    OptionString & proxy_sslclientkey();
    const OptionString & proxy_sslclientkey() const;
    OptionBool & deltarpm();
    const OptionBool & deltarpm() const;
    OptionNumber<std::uint32_t> & deltarpm_percentage();
    const OptionNumber<std::uint32_t> & deltarpm_percentage() const;
    OptionBool & skip_if_unavailable();
    const OptionBool & skip_if_unavailable() const;

    virtual void load_from_parser(
        const libdnf::ConfigParser & parser,
        const std::string & section,
        const libdnf::Vars & vars,
        libdnf::Logger & logger) override;

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf

#endif
