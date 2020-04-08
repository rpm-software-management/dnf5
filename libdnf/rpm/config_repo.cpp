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

#include "ConfigRepo.hpp"
#include "Const.hpp"
#include "Config-private.hpp"

namespace libdnf {

class ConfigRepo::Impl {
    friend class ConfigRepo;

    Impl(Config & owner, ConfigMain & masterConfig);

    Config & owner;
    ConfigMain & masterConfig;

    OptionString name{""};
    OptionChild<OptionBool> enabled{masterConfig.enabled()};
    OptionChild<OptionString> basecachedir{masterConfig.cachedir()};
    OptionStringList baseurl{std::vector<std::string>{}};
    OptionString mirrorlist{nullptr};
    OptionString metalink{nullptr};
    OptionString type{""};
    OptionString mediaid{""};
    OptionStringList gpgkey{std::vector<std::string>{}};
    OptionStringList excludepkgs{std::vector<std::string>{}};
    OptionStringList includepkgs{std::vector<std::string>{}};
    OptionChild<OptionBool> fastestmirror{masterConfig.fastestmirror()};
    OptionChild<OptionString> proxy{masterConfig.proxy()};
    OptionChild<OptionString> proxy_username{masterConfig.proxy_username()};
    OptionChild<OptionString> proxy_password{masterConfig.proxy_password()};
    OptionChild<OptionEnum<std::string> > proxy_auth_method{masterConfig.proxy_auth_method()};
    OptionChild<OptionString> username{masterConfig.username()};
    OptionChild<OptionString> password{masterConfig.password()};
    OptionChild<OptionStringList> protected_packages{masterConfig.protected_packages()};
    OptionChild<OptionBool> gpgcheck{masterConfig.gpgcheck()};
    OptionChild<OptionBool> repo_gpgcheck{masterConfig.repo_gpgcheck()};
    OptionChild<OptionBool> enablegroups{masterConfig.enablegroups()};
    OptionChild<OptionNumber<std::uint32_t> > retries{masterConfig.retries()};
    OptionChild<OptionNumber<std::uint32_t> > bandwidth{masterConfig.bandwidth()};
    OptionChild<OptionNumber<std::uint32_t> > minrate{masterConfig.minrate()};
    OptionChild<OptionEnum<std::string> > ip_resolve{masterConfig.ip_resolve()};
    OptionChild<OptionNumber<float> > throttle{masterConfig.throttle()};
    OptionChild<OptionSeconds> timeout{masterConfig.timeout()};
    OptionChild<OptionNumber<std::uint32_t> >  max_parallel_downloads{masterConfig.max_parallel_downloads()};
    OptionChild<OptionSeconds> metadata_expire{masterConfig.metadata_expire()};
    OptionNumber<std::int32_t> cost{1000};
    OptionNumber<std::int32_t> priority{99};
    OptionBool module_hotfixes{false};
    OptionChild<OptionString> sslcacert{masterConfig.sslcacert()};
    OptionChild<OptionBool> sslverify{masterConfig.sslverify()};
    OptionChild<OptionString> sslclientcert{masterConfig.sslclientcert()};
    OptionChild<OptionString> sslclientkey{masterConfig.sslclientkey()};
    OptionChild<OptionBool> deltarpm{masterConfig.deltarpm()};
    OptionChild<OptionNumber<std::uint32_t> > deltarpm_percentage{masterConfig.deltarpm_percentage()};
    OptionChild<OptionBool> skip_if_unavailable{masterConfig.skip_if_unavailable()};
    OptionString enabled_metadata{""};
    OptionChild<OptionString> user_agent{masterConfig.user_agent()};
    OptionChild<OptionBool> countme{masterConfig.countme()};
    OptionEnum<std::string> failovermethod{"priority", {"priority", "roundrobin"}};
};

ConfigRepo::Impl::Impl(Config & owner, ConfigMain & masterConfig)
: owner(owner), masterConfig(masterConfig)
{
    owner.optBinds().add("name", name);
    owner.optBinds().add("enabled", enabled);
    owner.optBinds().add("cachedir", basecachedir);
    owner.optBinds().add("baseurl", baseurl);
    owner.optBinds().add("mirrorlist", mirrorlist);
    owner.optBinds().add("metalink", metalink);
    owner.optBinds().add("type", type);
    owner.optBinds().add("mediaid", mediaid);
    owner.optBinds().add("gpgkey", gpgkey);

    owner.optBinds().add("excludepkgs", excludepkgs,
        [&](Option::Priority priority, const std::string & value){
            optionTListAppend(excludepkgs, priority, value);
        }, nullptr, true
    );
    owner.optBinds().add("exclude", excludepkgs, //compatibility with yum
        [&](Option::Priority priority, const std::string & value){
            optionTListAppend(excludepkgs, priority, value);
        }, nullptr, true
    );

    owner.optBinds().add("includepkgs", includepkgs,
        [&](Option::Priority priority, const std::string & value){
            optionTListAppend(includepkgs, priority, value);
        }, nullptr, true
    );

    owner.optBinds().add("fastestmirror", fastestmirror);

    owner.optBinds().add("proxy", proxy,
        [&](Option::Priority priority, const std::string & value){
            auto tmpValue(value);
            for (auto & ch : tmpValue)
                ch = std::tolower(ch);
            if (tmpValue == "_none_")
                proxy.set(priority, "");
            else
                proxy.set(priority, value);
        }, nullptr, false
    );

    owner.optBinds().add("proxy_username", proxy_username);
    owner.optBinds().add("proxy_password", proxy_password);
    owner.optBinds().add("proxy_auth_method", proxy_auth_method);
    owner.optBinds().add("username", username);
    owner.optBinds().add("password", password);
    owner.optBinds().add("protected_packages", protected_packages);
    owner.optBinds().add("gpgcheck", gpgcheck);
    owner.optBinds().add("repo_gpgcheck", repo_gpgcheck);
    owner.optBinds().add("enablegroups", enablegroups);
    owner.optBinds().add("retries", retries);
    owner.optBinds().add("bandwidth", bandwidth);
    owner.optBinds().add("minrate", minrate);
    owner.optBinds().add("ip_resolve", ip_resolve);
    owner.optBinds().add("throttle", throttle);
    owner.optBinds().add("timeout", timeout);
    owner.optBinds().add("max_parallel_downloads", max_parallel_downloads);
    owner.optBinds().add("metadata_expire", metadata_expire);
    owner.optBinds().add("cost", cost);
    owner.optBinds().add("priority", priority);
    owner.optBinds().add("module_hotfixes", module_hotfixes);
    owner.optBinds().add("sslcacert", sslcacert);
    owner.optBinds().add("sslverify", sslverify);
    owner.optBinds().add("sslclientcert", sslclientcert);
    owner.optBinds().add("sslclientkey", sslclientkey);
    owner.optBinds().add("deltarpm", deltarpm);
    owner.optBinds().add("deltarpm_percentage", deltarpm_percentage);
    owner.optBinds().add("skip_if_unavailable", skip_if_unavailable);
    owner.optBinds().add("enabled_metadata", enabled_metadata);
    owner.optBinds().add("user_agent", user_agent);
    owner.optBinds().add("countme", countme);
}

ConfigRepo::ConfigRepo(ConfigMain & masterConfig) : pImpl(new Impl(*this, masterConfig)) {}
ConfigRepo::~ConfigRepo() = default;
ConfigRepo::ConfigRepo(ConfigRepo && src) : pImpl(std::move(src.pImpl)) {}

ConfigMain & ConfigRepo::getMasterConfig() { return pImpl->masterConfig; }

OptionString & ConfigRepo::name() { return pImpl->name; }
OptionChild<OptionBool> & ConfigRepo::enabled() { return pImpl->enabled; }
OptionChild<OptionString> & ConfigRepo::basecachedir() { return pImpl->basecachedir; }
OptionStringList & ConfigRepo::baseurl() { return pImpl->baseurl; }
OptionString & ConfigRepo::mirrorlist() { return pImpl->mirrorlist; }
OptionString & ConfigRepo::metalink() { return pImpl->metalink; }
OptionString & ConfigRepo::type() { return pImpl->type; }
OptionString & ConfigRepo::mediaid() { return pImpl->mediaid; }
OptionStringList & ConfigRepo::gpgkey() { return pImpl->gpgkey; }
OptionStringList & ConfigRepo::excludepkgs() { return pImpl->excludepkgs; }
OptionStringList & ConfigRepo::includepkgs() { return pImpl->includepkgs; }
OptionChild<OptionBool> & ConfigRepo::fastestmirror() { return pImpl->fastestmirror; }
OptionChild<OptionString> & ConfigRepo::proxy() { return pImpl->proxy; }
OptionChild<OptionString> & ConfigRepo::proxy_username() { return pImpl->proxy_username; }
OptionChild<OptionString> & ConfigRepo::proxy_password() { return pImpl->proxy_password; }
OptionChild<OptionEnum<std::string> > & ConfigRepo::proxy_auth_method() { return pImpl->proxy_auth_method; }
OptionChild<OptionString> & ConfigRepo::username() { return pImpl->username; }
OptionChild<OptionString> & ConfigRepo::password() { return pImpl->password; }
OptionChild<OptionStringList> & ConfigRepo::protected_packages() { return pImpl->protected_packages; }
OptionChild<OptionBool> & ConfigRepo::gpgcheck() { return pImpl->gpgcheck; }
OptionChild<OptionBool> & ConfigRepo::repo_gpgcheck() { return pImpl->repo_gpgcheck; }
OptionChild<OptionBool> & ConfigRepo::enablegroups() { return pImpl->enablegroups; }
OptionChild<OptionNumber<std::uint32_t> > & ConfigRepo::retries() { return pImpl->retries; }
OptionChild<OptionNumber<std::uint32_t> > & ConfigRepo::bandwidth() { return pImpl->bandwidth; }
OptionChild<OptionNumber<std::uint32_t> > & ConfigRepo::minrate() { return pImpl->minrate; }
OptionChild<OptionEnum<std::string> > & ConfigRepo::ip_resolve() { return pImpl->ip_resolve; }
OptionChild<OptionNumber<float> > & ConfigRepo::throttle() { return pImpl->throttle; }
OptionChild<OptionSeconds> & ConfigRepo::timeout() { return pImpl->timeout; }
OptionChild<OptionNumber<std::uint32_t> > & ConfigRepo::max_parallel_downloads() { return pImpl->max_parallel_downloads; }
OptionChild<OptionSeconds> & ConfigRepo::metadata_expire() { return pImpl->metadata_expire; }
OptionNumber<std::int32_t> & ConfigRepo::cost() { return pImpl->cost; }
OptionNumber<std::int32_t> & ConfigRepo::priority() { return pImpl->priority; }
OptionBool & ConfigRepo::module_hotfixes() { return pImpl->module_hotfixes; }
OptionChild<OptionString> & ConfigRepo::sslcacert() { return pImpl->sslcacert; }
OptionChild<OptionBool> & ConfigRepo::sslverify() { return pImpl->sslverify; }
OptionChild<OptionString> & ConfigRepo::sslclientcert() { return pImpl->sslclientcert; }
OptionChild<OptionString> & ConfigRepo::sslclientkey() { return pImpl->sslclientkey; }
OptionChild<OptionBool> & ConfigRepo::deltarpm() { return pImpl->deltarpm; }
OptionChild<OptionNumber<std::uint32_t> > & ConfigRepo::deltarpm_percentage() { return pImpl->deltarpm_percentage; }
OptionChild<OptionBool> & ConfigRepo::skip_if_unavailable() { return pImpl->skip_if_unavailable; }
OptionString & ConfigRepo::enabled_metadata() { return pImpl->enabled_metadata; }
OptionChild<OptionString> & ConfigRepo::user_agent() { return pImpl->user_agent; }
OptionChild<OptionBool> & ConfigRepo::countme() { return pImpl->countme; }
OptionEnum<std::string> & ConfigRepo::failovermethod() { return pImpl->failovermethod; }

}
