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

#ifndef _LIBDNF_CONFIG_REPO_HPP
#define _LIBDNF_CONFIG_REPO_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "ConfigMain.hpp"
#include "OptionChild.hpp"

#include <memory>

namespace libdnf {

/**
* @class ConfigRepo
*
* @brief Holds repo configuration options
*
* Default values of some options are inherited from ConfigMain.
* 
*/
class ConfigRepo : public Config {
public:
    ConfigRepo(ConfigMain & masterConfig);
    ~ConfigRepo();
    ConfigRepo(ConfigRepo && src);

    ConfigMain & getMasterConfig();

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
    OptionChild<OptionEnum<std::string> > & proxy_auth_method();
    OptionChild<OptionString> & username();
    OptionChild<OptionString> & password();
    OptionChild<OptionStringList> & protected_packages();
    OptionChild<OptionBool> & gpgcheck();
    OptionChild<OptionBool> & repo_gpgcheck();
    OptionChild<OptionBool> & enablegroups();
    OptionChild<OptionNumber<std::uint32_t> > & retries();
    OptionChild<OptionNumber<std::uint32_t> > & bandwidth();
    OptionChild<OptionNumber<std::uint32_t> > & minrate();
    OptionChild<OptionEnum<std::string> > & ip_resolve();
    OptionChild<OptionNumber<float> > & throttle();
    OptionChild<OptionSeconds> & timeout();
    OptionChild<OptionNumber<std::uint32_t> > & max_parallel_downloads();
    OptionChild<OptionSeconds> & metadata_expire();
    OptionNumber<std::int32_t> & cost();
    OptionNumber<std::int32_t> & priority();
    OptionBool & module_hotfixes();
    OptionChild<OptionString> & sslcacert();
    OptionChild<OptionBool> & sslverify();
    OptionChild<OptionString> & sslclientcert();
    OptionChild<OptionString> & sslclientkey();
    OptionChild<OptionBool> & deltarpm();
    OptionChild<OptionNumber<std::uint32_t> > & deltarpm_percentage();
    OptionChild<OptionBool> & skip_if_unavailable();
    // option recognized by other tools, e.g. gnome-software, but unused in dnf
    OptionString & enabled_metadata();
    OptionChild<OptionString> & user_agent();
    OptionChild<OptionBool> & countme();
    // yum compatibility options
    OptionEnum<std::string> & failovermethod();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif

#endif
