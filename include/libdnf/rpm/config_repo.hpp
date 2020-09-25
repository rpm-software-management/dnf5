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

#ifndef LIBDNF_RPM_CONFIG_REPO_HPP
#define LIBDNF_RPM_CONFIG_REPO_HPP

#include "libdnf/conf/config_main.hpp"
#include "libdnf/conf/option_child.hpp"

#include <memory>

namespace libdnf::rpm {

/// Holds repo configuration options. Default values of some options are inherited from ConfigMain.
class ConfigRepo : public Config<Option::Priority::REPOCONFIG> {
public:
    explicit ConfigRepo(ConfigMain & master_config);
    ~ConfigRepo();
    ConfigRepo(ConfigRepo && src);

    ConfigMain & get_master_config();

    OptionString & name();
    OptionChild<OptionBool> & enabled();
    OptionChild<OptionString> & basecachedir();
    OptionStringList & baseurl();
    OptionString & mirrorlist();
    OptionString & metalink();
    OptionString & type();
    OptionString & mediaid();
    OptionStringList & gpgkey();
    OptionStringList & excludepkgs();
    OptionStringList & includepkgs();
    OptionChild<OptionBool> & fastestmirror();
    OptionChild<OptionString> & proxy();
    OptionChild<OptionString> & proxy_username();
    OptionChild<OptionString> & proxy_password();
    OptionChild<OptionEnum<std::string>> & proxy_auth_method();
    OptionChild<OptionString> & username();
    OptionChild<OptionString> & password();
    OptionChild<OptionStringList> & protected_packages();
    OptionChild<OptionBool> & gpgcheck();
    OptionChild<OptionBool> & repo_gpgcheck();
    OptionChild<OptionBool> & enablegroups();
    OptionChild<OptionNumber<std::uint32_t>> & retries();
    OptionChild<OptionNumber<std::uint32_t>> & bandwidth();
    OptionChild<OptionNumber<std::uint32_t>> & minrate();
    OptionChild<OptionEnum<std::string>> & ip_resolve();
    OptionChild<OptionNumber<float>> & throttle();
    OptionChild<OptionSeconds> & timeout();
    OptionChild<OptionNumber<std::uint32_t>> & max_parallel_downloads();
    OptionChild<OptionSeconds> & metadata_expire();
    OptionNumber<std::int32_t> & cost();
    OptionNumber<std::int32_t> & priority();
    OptionBool & module_hotfixes();
    OptionChild<OptionString> & sslcacert();
    OptionChild<OptionBool> & sslverify();
    OptionChild<OptionString> & sslclientcert();
    OptionChild<OptionString> & sslclientkey();
    OptionChild<OptionBool> & deltarpm();
    OptionChild<OptionNumber<std::uint32_t>> & deltarpm_percentage();
    OptionChild<OptionBool> & skip_if_unavailable();
    /// If true it will create libsolv cache that will speed up the next loading process
    OptionBool & build_cache();

    // option recognized by other tools, e.g. gnome-software, but unused in dnf
    OptionString & enabled_metadata();

    OptionChild<OptionString> & user_agent();
    OptionChild<OptionBool> & countme();
    // yum compatibility options
    OptionEnum<std::string> & failovermethod();

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf::rpm

#endif
