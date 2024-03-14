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

#ifndef LIBDNF5_RPM_CONFIG_REPO_HPP
#define LIBDNF5_RPM_CONFIG_REPO_HPP

#include "libdnf5/conf/config_main.hpp"
#include "libdnf5/conf/option_child.hpp"
#include "libdnf5/defs.h"

#include <memory>


namespace libdnf5::repo {

/// Holds repo configuration options. Default values of some options are inherited from ConfigMain.
class LIBDNF_API ConfigRepo : public Config {
public:
    ConfigRepo(ConfigMain & main_config, const std::string & id);
    ~ConfigRepo();
    ConfigRepo(ConfigRepo && src);

    ConfigMain & get_main_config();
    const ConfigMain & get_main_config() const;

    std::string get_id() const;

    OptionString & get_name_option();
    const OptionString & get_name_option() const;
    OptionChild<OptionBool> & get_enabled_option();
    const OptionChild<OptionBool> & get_enabled_option() const;
    OptionChild<OptionString> & get_basecachedir_option();
    const OptionChild<OptionString> & get_basecachedir_option() const;
    OptionStringList & get_baseurl_option();
    const OptionStringList & get_baseurl_option() const;
    OptionString & get_mirrorlist_option();
    const OptionString & get_mirrorlist_option() const;
    OptionString & get_metalink_option();
    const OptionString & get_metalink_option() const;
    OptionString & get_type_option();
    const OptionString & get_type_option() const;
    OptionString & get_mediaid_option();
    const OptionString & get_mediaid_option() const;
    OptionStringList & get_gpgkey_option();
    const OptionStringList & get_gpgkey_option() const;
    OptionStringAppendList & get_excludepkgs_option();
    const OptionStringAppendList & get_excludepkgs_option() const;
    OptionStringAppendList & get_includepkgs_option();
    const OptionStringAppendList & get_includepkgs_option() const;
    OptionChild<OptionBool> & get_fastestmirror_option();
    const OptionChild<OptionBool> & get_fastestmirror_option() const;
    OptionChild<OptionString> & get_proxy_option();
    const OptionChild<OptionString> & get_proxy_option() const;
    OptionChild<OptionString> & get_proxy_username_option();
    const OptionChild<OptionString> & get_proxy_username_option() const;
    OptionChild<OptionString> & get_proxy_password_option();
    const OptionChild<OptionString> & get_proxy_password_option() const;
    OptionChild<OptionStringSet> & get_proxy_auth_method_option();
    const OptionChild<OptionStringSet> & get_proxy_auth_method_option() const;
    OptionChild<OptionString> & get_username_option();
    const OptionChild<OptionString> & get_username_option() const;
    OptionChild<OptionString> & get_password_option();
    const OptionChild<OptionString> & get_password_option() const;
    OptionChild<OptionStringAppendList> & get_protected_packages_option();
    const OptionChild<OptionStringAppendList> & get_protected_packages_option() const;
    OptionChild<OptionBool> & get_gpgcheck_option();
    const OptionChild<OptionBool> & get_gpgcheck_option() const;
    OptionChild<OptionBool> & get_repo_gpgcheck_option();
    const OptionChild<OptionBool> & get_repo_gpgcheck_option() const;
    OptionChild<OptionBool> & get_enablegroups_option();
    const OptionChild<OptionBool> & get_enablegroups_option() const;
    OptionChild<OptionNumber<std::uint32_t>> & get_retries_option();
    const OptionChild<OptionNumber<std::uint32_t>> & get_retries_option() const;
    OptionChild<OptionNumber<std::uint32_t>> & get_bandwidth_option();
    const OptionChild<OptionNumber<std::uint32_t>> & get_bandwidth_option() const;
    OptionChild<OptionNumber<std::uint32_t>> & get_minrate_option();
    const OptionChild<OptionNumber<std::uint32_t>> & get_minrate_option() const;
    OptionChild<OptionEnum> & get_ip_resolve_option();
    const OptionChild<OptionEnum> & get_ip_resolve_option() const;
    OptionChild<OptionNumber<float>> & get_throttle_option();
    const OptionChild<OptionNumber<float>> & get_throttle_option() const;
    OptionChild<OptionSeconds> & get_timeout_option();
    const OptionChild<OptionSeconds> & get_timeout_option() const;
    OptionChild<OptionNumber<std::uint32_t>> & get_max_parallel_downloads_option();
    const OptionChild<OptionNumber<std::uint32_t>> & get_max_parallel_downloads_option() const;
    OptionChild<OptionSeconds> & get_metadata_expire_option();
    const OptionChild<OptionSeconds> & get_metadata_expire_option() const;
    OptionNumber<std::int32_t> & get_cost_option();
    const OptionNumber<std::int32_t> & get_cost_option() const;
    OptionNumber<std::int32_t> & get_priority_option();
    const OptionNumber<std::int32_t> & get_priority_option() const;
    OptionBool & get_module_hotfixes_option();
    const OptionBool & get_module_hotfixes_option() const;
    OptionChild<OptionString> & get_sslcacert_option();
    const OptionChild<OptionString> & get_sslcacert_option() const;
    OptionChild<OptionBool> & get_sslverify_option();
    const OptionChild<OptionBool> & get_sslverify_option() const;
    OptionChild<OptionString> & get_sslclientcert_option();
    const OptionChild<OptionString> & get_sslclientcert_option() const;
    OptionChild<OptionString> & get_sslclientkey_option();
    const OptionChild<OptionString> & get_sslclientkey_option() const;
    OptionChild<OptionString> & get_proxy_sslcacert_option();
    const OptionChild<OptionString> & get_proxy_sslcacert_option() const;
    OptionChild<OptionBool> & get_proxy_sslverify_option();
    const OptionChild<OptionBool> & get_proxy_sslverify_option() const;
    OptionChild<OptionString> & get_proxy_sslclientcert_option();
    const OptionChild<OptionString> & get_proxy_sslclientcert_option() const;
    OptionChild<OptionString> & get_proxy_sslclientkey_option();
    const OptionChild<OptionString> & get_proxy_sslclientkey_option() const;
    OptionChild<OptionBool> & get_deltarpm_option();
    const OptionChild<OptionBool> & get_deltarpm_option() const;
    OptionChild<OptionNumber<std::uint32_t>> & get_deltarpm_percentage_option();
    const OptionChild<OptionNumber<std::uint32_t>> & get_deltarpm_percentage_option() const;
    OptionChild<OptionBool> & get_skip_if_unavailable_option();
    const OptionChild<OptionBool> & get_skip_if_unavailable_option() const;
    /// If true it will create libsolv cache that will speed up the next loading process
    OptionChild<OptionBool> & get_build_cache_option();
    const OptionChild<OptionBool> & get_build_cache_option() const;

    // option recognized by other tools, e.g. gnome-software, but unused in dnf
    OptionString & get_enabled_metadata_option();
    const OptionString & get_enabled_metadata_option() const;

    OptionChild<OptionString> & get_user_agent_option();
    const OptionChild<OptionString> & get_user_agent_option() const;
    OptionChild<OptionBool> & get_countme_option();
    const OptionChild<OptionBool> & get_countme_option() const;
    // yum compatibility options
    OptionEnum & get_failovermethod_option();
    const OptionEnum & get_failovermethod_option() const;

    /// @return A unique ID of the repository, consisting of its id and a hash
    /// computed from its source URLs (metalink, mirrorlist or baseurl, first
    /// one set is used in the order listed).
    std::string get_unique_id() const;

    /// @return The path to the repository's cache directory, where its
    /// cached metadata are stored. The path contains unique ID generated by get_unique_id()
    std::string get_cachedir() const;

    /// @return The path to the repository's persistent directory, where its
    /// persistent data are stored.
    std::string get_persistdir() const;

    void load_from_parser(
        const libdnf5::ConfigParser & parser,
        const std::string & section,
        const libdnf5::Vars & vars,
        libdnf5::Logger & logger,
        Option::Priority priority = Option::Priority::REPOCONFIG) override;

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::repo

#endif
