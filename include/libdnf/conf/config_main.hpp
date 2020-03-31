/*
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LIBDNF_CONFIG_MAIN_HPP
#define _LIBDNF_CONFIG_MAIN_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "Config.hpp"
#include "OptionBool.hpp"
#include "OptionEnum.hpp"
#include "OptionNumber.hpp"
#include "OptionPath.hpp"
#include "OptionSeconds.hpp"
#include "OptionString.hpp"
#include "OptionStringList.hpp"

#include <memory>

namespace libdnf {

/**
* @class ConfigMain
*
* @brief Holds configuration options needed for libdnf
*
*/
class ConfigMain : public Config {
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

    /*  NOTE: If you set this to 2, then because it keeps the current
    kernel it means if you ever install an "old" kernel it'll get rid
    of the newest one so you probably want to use 3 as a minimum
    ... if you turn it on. */
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
    OptionSeconds & metadata_timer_sync();
    OptionStringList & disable_excludes();
    OptionEnum<std::string> & multilib_policy(); // :api
    OptionBool & best(); // :api
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
    OptionBool & strict(); // :api
    OptionBool & skip_broken(); // :yum-compatibility
    OptionBool & autocheck_running_kernel(); // :yum-compatibility
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

    /**
    * @brief Adds variables from directory
    *
    * Additional variables are added from directory. Each file represents one variable.
    * The variable name is equal to filename and the value is defined by first line of the file.
    *
    * @param varsMap Storage where the variables are added.
    * @param dirPath Path to directory.
    */
    static void addVarsFromDir(std::map<std::string, std::string> & varsMap, const std::string & dirPath);

    /**
    * @brief Adds variables from environment
    *
    * Environment variables DNF[0-9] and DNF_VAR_[A-Za-z0-9_]+ are used if exists.
    * DNF_VAR_ prefix is cut off.
    *
    * @param varsMap Storage where the variables are added.
    */
    static void addVarsFromEnv(std::map<std::string, std::string> & varsMap);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif

#endif
