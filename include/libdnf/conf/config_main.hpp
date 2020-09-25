/*
Copyright (C) 2018-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
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
    OptionNumber<std::int32_t> & errorlevel();
    OptionString & installroot();
    OptionString & config_file_path();
    OptionBool & plugins();
    OptionStringList & pluginpath();
    OptionStringList & pluginconfpath();
    OptionString & persistdir();
    OptionBool & transformdb();
    OptionNumber<std::int32_t> & recent();
    OptionBool & reset_nice();
    OptionString & system_cachedir();
    OptionBool & cacheonly();
    OptionBool & keepcache();
    OptionString & logdir();
    OptionNumber<std::int32_t> & log_size();
    OptionNumber<std::int32_t> & log_rotate();
    OptionStringList & varsdir();
    OptionStringList & reposdir();
    OptionBool & debug_solver();
    OptionStringList & installonlypkgs();
    OptionStringList & group_package_types();

    //  NOTE: If you set this to 2, then because it keeps the current
    // kernel it means if you ever install an "old" kernel it'll get rid
    // of the newest one so you probably want to use 3 as a minimum
    // ... if you turn it on.
    OptionNumber<std::uint32_t> & installonly_limit();

    OptionStringList & tsflags();
    OptionBool & assumeyes();
    OptionBool & assumeno();
    OptionBool & check_config_file_age();
    OptionBool & defaultyes();
    OptionBool & diskspacecheck();
    OptionBool & localpkg_gpgcheck();
    OptionBool & gpgkey_dns_verification();
    OptionBool & obsoletes();
    OptionBool & showdupesfromrepos();
    OptionBool & exit_on_lock();
    OptionBool & allow_vendor_change();
    OptionSeconds & metadata_timer_sync();
    OptionStringList & disable_excludes();
    OptionEnum<std::string> & multilib_policy();  // :api
    OptionBool & best();                          // :api
    OptionBool & install_weak_deps();
    OptionString & bugtracker_url();
    OptionBool & zchunk();
    OptionEnum<std::string> & color();
    OptionString & color_list_installed_older();
    OptionString & color_list_installed_newer();
    OptionString & color_list_installed_reinstall();
    OptionString & color_list_installed_extra();
    OptionString & color_list_available_upgrade();
    OptionString & color_list_available_downgrade();
    OptionString & color_list_available_reinstall();
    OptionString & color_list_available_install();
    OptionString & color_update_installed();
    OptionString & color_update_local();
    OptionString & color_update_remote();
    OptionString & color_search_match();
    OptionBool & history_record();
    OptionStringList & history_record_packages();
    OptionString & rpmverbosity();
    OptionBool & strict();                    // :api
    OptionBool & skip_broken();               // :yum-compatibility
    OptionBool & autocheck_running_kernel();  // :yum-compatibility
    OptionBool & clean_requirements_on_remove();
    OptionEnum<std::string> & history_list_view();
    OptionBool & upgrade_group_objects_upgrade();
    OptionPath & destdir();
    OptionString & comment();
    OptionBool & downloadonly();
    OptionBool & ignorearch();

    OptionString & module_platform_id();
    OptionString & user_agent();
    OptionBool & countme();

    // Repo main config
    OptionNumber<std::uint32_t> & retries();
    OptionString & cachedir();
    OptionBool & fastestmirror();
    OptionStringList & excludepkgs();
    OptionStringList & includepkgs();
    OptionString & proxy();
    OptionString & proxy_username();
    OptionString & proxy_password();
    OptionEnum<std::string> & proxy_auth_method();
    OptionStringList & protected_packages();
    OptionString & username();
    OptionString & password();
    OptionBool & gpgcheck();
    OptionBool & repo_gpgcheck();
    OptionBool & enabled();
    OptionBool & enablegroups();
    OptionNumber<std::uint32_t> & bandwidth();
    OptionNumber<std::uint32_t> & minrate();
    OptionEnum<std::string> & ip_resolve();
    OptionNumber<float> & throttle();
    OptionSeconds & timeout();
    OptionNumber<std::uint32_t> & max_parallel_downloads();
    OptionSeconds & metadata_expire();
    OptionString & sslcacert();
    OptionBool & sslverify();
    OptionString & sslclientcert();
    OptionString & sslclientkey();
    OptionBool & deltarpm();
    OptionNumber<std::uint32_t> & deltarpm_percentage();
    OptionBool & skip_if_unavailable();

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf

#endif
