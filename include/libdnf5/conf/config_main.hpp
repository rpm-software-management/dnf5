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

#ifndef LIBDNF5_CONF_CONFIG_MAIN_HPP
#define LIBDNF5_CONF_CONFIG_MAIN_HPP

#include "config.hpp"
#include "option_bool.hpp"
#include "option_enum.hpp"
#include "option_number.hpp"
#include "option_path.hpp"
#include "option_seconds.hpp"
#include "option_string.hpp"
#include "option_string_list.hpp"

#include "libdnf5/logger/logger.hpp"

#include <memory>


namespace libdnf5 {

/// Holds global configuration
class LIBDNF_API ConfigMain : public Config {
public:
    ConfigMain();
    ~ConfigMain();

    OptionNumber<std::int32_t> & get_debuglevel_option();
    const OptionNumber<std::int32_t> & get_debuglevel_option() const;
    /// @deprecated The option does nothing
    [[deprecated("The option does nothing")]] OptionNumber<std::int32_t> & get_errorlevel_option();
    /// @deprecated The option does nothing
    [[deprecated("The option does nothing")]] const OptionNumber<std::int32_t> & get_errorlevel_option() const;
    OptionPath & get_installroot_option();
    const OptionPath & get_installroot_option() const;
    OptionPath & get_config_file_path_option();
    const OptionPath & get_config_file_path_option() const;
    OptionBool & get_plugins_option();
    const OptionBool & get_plugins_option() const;
    OptionPath & get_pluginpath_option();
    const OptionPath & get_pluginpath_option() const;
    OptionPath & get_pluginconfpath_option();
    const OptionPath & get_pluginconfpath_option() const;
    OptionPath & get_persistdir_option();
    const OptionPath & get_persistdir_option() const;
    OptionPath & get_system_state_dir_option();
    const OptionPath & get_system_state_dir_option() const;
    OptionPath & get_transaction_history_dir_option();
    const OptionPath & get_transaction_history_dir_option() const;
    OptionBool & get_transformdb_option();
    const OptionBool & get_transformdb_option() const;
    OptionNumber<std::int32_t> & get_recent_option();
    const OptionNumber<std::int32_t> & get_recent_option() const;
    OptionBool & get_reset_nice_option();
    const OptionBool & get_reset_nice_option() const;
    OptionPath & get_system_cachedir_option();
    const OptionPath & get_system_cachedir_option() const;
    OptionEnum & get_cacheonly_option();
    const OptionEnum & get_cacheonly_option() const;
    OptionBool & get_keepcache_option();
    const OptionBool & get_keepcache_option() const;
    OptionPath & get_logdir_option();
    const OptionPath & get_logdir_option() const;
    OptionNumber<std::int32_t> & get_log_size_option();
    const OptionNumber<std::int32_t> & get_log_size_option() const;
    OptionNumber<std::int32_t> & get_log_rotate_option();
    const OptionNumber<std::int32_t> & get_log_rotate_option() const;
    OptionPath & get_debugdir_option();
    const OptionPath & get_debugdir_option() const;
    OptionStringList & get_varsdir_option();
    const OptionStringList & get_varsdir_option() const;
    OptionStringList & get_reposdir_option();
    const OptionStringList & get_reposdir_option() const;
    OptionBool & get_debug_solver_option();
    const OptionBool & get_debug_solver_option() const;
    OptionStringAppendList & get_installonlypkgs_option();
    const OptionStringAppendList & get_installonlypkgs_option() const;
    OptionStringList & get_group_package_types_option();
    const OptionStringList & get_group_package_types_option() const;
    OptionStringAppendSet & get_optional_metadata_types_option();
    const OptionStringAppendSet & get_optional_metadata_types_option() const;
    OptionBool & get_use_host_config_option();
    const OptionBool & get_use_host_config_option() const;

    //  NOTE: If you set this to 2, then because it keeps the current
    // kernel it means if you ever install an "old" kernel it'll get rid
    // of the newest one so you probably want to use 3 as a minimum
    // ... if you turn it on.
    OptionNumber<std::uint32_t> & get_installonly_limit_option();
    const OptionNumber<std::uint32_t> & get_installonly_limit_option() const;

    OptionStringAppendList & get_tsflags_option();
    const OptionStringAppendList & get_tsflags_option() const;
    OptionBool & get_assumeyes_option();
    const OptionBool & get_assumeyes_option() const;
    OptionBool & get_assumeno_option();
    const OptionBool & get_assumeno_option() const;
    OptionBool & get_check_config_file_age_option();
    const OptionBool & get_check_config_file_age_option() const;
    OptionBool & get_defaultyes_option();
    const OptionBool & get_defaultyes_option() const;
    OptionBool & get_diskspacecheck_option();
    const OptionBool & get_diskspacecheck_option() const;
    OptionBool & get_localpkg_gpgcheck_option();
    const OptionBool & get_localpkg_gpgcheck_option() const;
    OptionBool & get_gpgkey_dns_verification_option();
    const OptionBool & get_gpgkey_dns_verification_option() const;
    OptionBool & get_obsoletes_option();
    const OptionBool & get_obsoletes_option() const;
    OptionBool & get_exit_on_lock_option();
    const OptionBool & get_exit_on_lock_option() const;
    OptionBool & get_allow_vendor_change_option();
    const OptionBool & get_allow_vendor_change_option() const;
    /// @deprecated The metadata_timer_sync option does nothing
    [[deprecated("The metadata_timer_sync option does nothing")]] OptionSeconds & get_metadata_timer_sync_option();
    /// @deprecated The metadata_timer_sync option does nothing
    [[deprecated("The metadata_timer_sync option does nothing")]] const OptionSeconds & get_metadata_timer_sync_option()
        const;
    OptionStringList & get_disable_excludes_option();
    const OptionStringList & get_disable_excludes_option() const;
    OptionEnum & get_multilib_policy_option();  // :api
    const OptionEnum & get_multilib_policy_option() const;
    OptionBool & get_best_option();  // :api
    const OptionBool & get_best_option() const;
    OptionBool & get_install_weak_deps_option();
    const OptionBool & get_install_weak_deps_option() const;
    OptionBool & get_allow_downgrade_option();
    const OptionBool & get_allow_downgrade_option() const;
    OptionString & get_bugtracker_url_option();
    const OptionString & get_bugtracker_url_option() const;
    OptionBool & get_zchunk_option();
    const OptionBool & get_zchunk_option() const;
    OptionEnum & get_color_option();
    const OptionEnum & get_color_option() const;
    OptionString & get_color_list_installed_older_option();
    const OptionString & get_color_list_installed_older_option() const;
    OptionString & get_color_list_installed_newer_option();
    const OptionString & get_color_list_installed_newer_option() const;
    OptionString & get_color_list_installed_reinstall_option();
    const OptionString & get_color_list_installed_reinstall_option() const;
    OptionString & get_color_list_installed_extra_option();
    const OptionString & get_color_list_installed_extra_option() const;
    OptionString & get_color_list_available_upgrade_option();
    const OptionString & get_color_list_available_upgrade_option() const;
    OptionString & get_color_list_available_downgrade_option();
    const OptionString & get_color_list_available_downgrade_option() const;
    OptionString & get_color_list_available_reinstall_option();
    const OptionString & get_color_list_available_reinstall_option() const;
    OptionString & get_color_list_available_install_option();
    const OptionString & get_color_list_available_install_option() const;
    OptionString & get_color_update_installed_option();
    const OptionString & get_color_update_installed_option() const;
    OptionString & get_color_update_local_option();
    const OptionString & get_color_update_local_option() const;
    OptionString & get_color_update_remote_option();
    const OptionString & get_color_update_remote_option() const;
    OptionString & get_color_search_match_option();
    const OptionString & get_color_search_match_option() const;
    OptionBool & get_history_record_option();
    const OptionBool & get_history_record_option() const;
    OptionStringList & get_history_record_packages_option();
    const OptionStringList & get_history_record_packages_option() const;
    OptionString & get_rpmverbosity_option();
    const OptionString & get_rpmverbosity_option() const;
    /// @deprecated Use get_skip_broken_option() and get_skip_unavailable_option()
    [[deprecated("Use get_skip_broken_option() and get_skip_unavailable_option()")]]
    OptionBool & get_strict_option();
    /// @deprecated Use get_skip_broken_option() const and get_skip_unavailable_option() const
    [[deprecated("Use get_skip_broken_option() const and get_skip_unavailable_option() const")]]
    const OptionBool & get_strict_option() const;
    /// Solver is allowed to skip transaction packages with broken dependencies
    OptionBool & get_skip_broken_option();
    const OptionBool & get_skip_broken_option() const;
    /// Solver is allowed to skip packages that are not available in repositories
    OptionBool & get_skip_unavailable_option();
    const OptionBool & get_skip_unavailable_option() const;
    OptionBool & get_autocheck_running_kernel_option();  // :yum-compatibility
    const OptionBool & get_autocheck_running_kernel_option() const;
    OptionBool & get_clean_requirements_on_remove_option();
    const OptionBool & get_clean_requirements_on_remove_option() const;
    OptionEnum & get_history_list_view_option();
    const OptionEnum & get_history_list_view_option() const;
    OptionBool & get_upgrade_group_objects_upgrade_option();
    const OptionBool & get_upgrade_group_objects_upgrade_option() const;
    OptionPath & get_destdir_option();
    const OptionPath & get_destdir_option() const;
    OptionString & get_comment_option();
    const OptionString & get_comment_option() const;
    OptionBool & get_downloadonly_option();
    const OptionBool & get_downloadonly_option() const;
    OptionBool & get_ignorearch_option();
    const OptionBool & get_ignorearch_option() const;

    OptionString & get_module_platform_id_option();
    const OptionString & get_module_platform_id_option() const;
    OptionBool & get_module_stream_switch_option();
    const OptionBool & get_module_stream_switch_option() const;
    OptionBool & get_module_obsoletes_option();
    const OptionBool & get_module_obsoletes_option() const;
    OptionString & get_user_agent_option();
    const OptionString & get_user_agent_option() const;
    OptionBool & get_countme_option();
    const OptionBool & get_countme_option() const;
    OptionBool & get_protect_running_kernel_option();
    const OptionBool & get_protect_running_kernel_option() const;
    OptionBool & get_build_cache_option();
    const OptionBool & get_build_cache_option() const;

    // Repo main config
    /// @deprecated The option does nothing
    [[deprecated("The option does nothing")]] OptionNumber<std::uint32_t> & get_retries_option();
    /// @deprecated The option does nothing
    [[deprecated("The option does nothing")]] const OptionNumber<std::uint32_t> & get_retries_option() const;
    OptionPath & get_cachedir_option();
    const OptionPath & get_cachedir_option() const;
    OptionBool & get_fastestmirror_option();
    const OptionBool & get_fastestmirror_option() const;
    OptionStringAppendList & get_excludeenvs_option();
    const OptionStringAppendList & get_excludeenvs_option() const;
    OptionStringAppendList & get_excludegroups_option();
    const OptionStringAppendList & get_excludegroups_option() const;
    OptionStringAppendList & get_excludepkgs_option();
    const OptionStringAppendList & get_excludepkgs_option() const;
    OptionStringAppendList & get_includepkgs_option();
    const OptionStringAppendList & get_includepkgs_option() const;
    OptionStringAppendList & get_exclude_from_weak_option();
    const OptionStringAppendList & get_exclude_from_weak_option() const;
    OptionBool & get_exclude_from_weak_autodetect_option();
    const OptionBool & get_exclude_from_weak_autodetect_option() const;
    OptionString & get_proxy_option();
    const OptionString & get_proxy_option() const;
    OptionString & get_proxy_username_option();
    const OptionString & get_proxy_username_option() const;
    OptionString & get_proxy_password_option();
    const OptionString & get_proxy_password_option() const;
    OptionStringSet & get_proxy_auth_method_option();
    const OptionStringSet & get_proxy_auth_method_option() const;
    OptionStringAppendList & get_protected_packages_option();
    const OptionStringAppendList & get_protected_packages_option() const;
    OptionString & get_username_option();
    const OptionString & get_username_option() const;
    OptionString & get_password_option();
    const OptionString & get_password_option() const;
    /// @deprecated Use get_pkg_gpgcheck_option()
    [[deprecated("Use get_pkg_gpgcheck_option()")]]
    OptionBool & get_gpgcheck_option();
    /// @deprecated Use get_pkg_gpgcheck_option() const
    [[deprecated("Use get_pkg_gpgcheck_option() const")]]
    const OptionBool & get_gpgcheck_option() const;
    OptionBool & get_pkg_gpgcheck_option();
    const OptionBool & get_pkg_gpgcheck_option() const;
    OptionBool & get_repo_gpgcheck_option();
    const OptionBool & get_repo_gpgcheck_option() const;
    OptionBool & get_enabled_option();
    const OptionBool & get_enabled_option() const;
    OptionBool & get_enablegroups_option();
    const OptionBool & get_enablegroups_option() const;
    OptionNumber<std::uint32_t> & get_bandwidth_option();
    const OptionNumber<std::uint32_t> & get_bandwidth_option() const;
    OptionNumber<std::uint32_t> & get_minrate_option();
    const OptionNumber<std::uint32_t> & get_minrate_option() const;
    OptionEnum & get_ip_resolve_option();
    const OptionEnum & get_ip_resolve_option() const;
    OptionNumber<float> & get_throttle_option();
    const OptionNumber<float> & get_throttle_option() const;
    OptionSeconds & get_timeout_option();
    const OptionSeconds & get_timeout_option() const;
    OptionNumber<std::uint32_t> & get_max_parallel_downloads_option();
    const OptionNumber<std::uint32_t> & get_max_parallel_downloads_option() const;
    OptionSeconds & get_metadata_expire_option();
    const OptionSeconds & get_metadata_expire_option() const;
    OptionString & get_sslcacert_option();
    const OptionString & get_sslcacert_option() const;
    OptionBool & get_sslverify_option();
    const OptionBool & get_sslverify_option() const;
    OptionString & get_sslclientcert_option();
    const OptionString & get_sslclientcert_option() const;
    OptionString & get_sslclientkey_option();
    const OptionString & get_sslclientkey_option() const;
    OptionString & get_proxy_sslcacert_option();
    const OptionString & get_proxy_sslcacert_option() const;
    OptionBool & get_proxy_sslverify_option();
    const OptionBool & get_proxy_sslverify_option() const;
    OptionString & get_proxy_sslclientcert_option();
    const OptionString & get_proxy_sslclientcert_option() const;
    OptionString & get_proxy_sslclientkey_option();
    const OptionString & get_proxy_sslclientkey_option() const;
    /// @deprecated The option does nothing
    [[deprecated("The option does nothing")]] OptionBool & get_deltarpm_option();
    /// @deprecated The option does nothing
    [[deprecated("The option does nothing")]] const OptionBool & get_deltarpm_option() const;
    /// @deprecated The option does nothing
    [[deprecated("The option does nothing")]] OptionNumber<std::uint32_t> & get_deltarpm_percentage_option();
    /// @deprecated The option does nothing
    [[deprecated("The option does nothing")]] const OptionNumber<std::uint32_t> & get_deltarpm_percentage_option()
        const;
    OptionBool & get_skip_if_unavailable_option();
    const OptionBool & get_skip_if_unavailable_option() const;

    void load_from_parser(
        const libdnf5::ConfigParser & parser,
        const std::string & section,
        const libdnf5::Vars & vars,
        libdnf5::Logger & logger,
        Option::Priority priority = Option::Priority::MAINCONFIG) override;

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
