// Copyright Contributors to the DNF5 project.
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

#include "libdnf5/repo/config_repo.hpp"

#include "conf/config_utils.hpp"
#include "utils/deprecate.hpp"

#include "libdnf5/conf/const.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <solv/chksum.h>
#include <solv/util.h>

#include <filesystem>

namespace libdnf5::repo {

class ConfigRepo::Impl {
    friend class ConfigRepo;

    Impl(Config & owner, ConfigMain & main_config, const std::string & id);

    ConfigMain & main_config;
    std::string id;

    OptionString name{""};
    OptionBool enabled{true};
    OptionChild<OptionString> basecachedir{main_config.get_cachedir_option()};
    OptionStringList baseurl{std::vector<std::string>{}};
    OptionString mirrorlist{nullptr};
    OptionString metalink{nullptr};
    OptionString type{""};
    OptionString mediaid{""};
    OptionStringList gpgkey{std::vector<std::string>{}};
    OptionStringAppendList excludepkgs{std::vector<std::string>{}};
    OptionStringAppendList includepkgs{std::vector<std::string>{}};
    OptionChild<OptionBool> fastestmirror{main_config.get_fastestmirror_option()};
    OptionChild<OptionString> proxy{main_config.get_proxy_option()};
    OptionChild<OptionString> proxy_username{main_config.get_proxy_username_option()};
    OptionChild<OptionString> proxy_password{main_config.get_proxy_password_option()};
    OptionChild<OptionStringSet> proxy_auth_method{main_config.get_proxy_auth_method_option()};
    OptionChild<OptionString> username{main_config.get_username_option()};
    OptionChild<OptionString> password{main_config.get_password_option()};
    OptionChild<OptionStringAppendList> protected_packages{main_config.get_protected_packages_option()};
    OptionChild<OptionBool> pkg_gpgcheck{main_config.get_pkg_gpgcheck_option()};
    OptionChild<OptionBool> repo_gpgcheck{main_config.get_repo_gpgcheck_option()};
    OptionChild<OptionBool> enablegroups{main_config.get_enablegroups_option()};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    OptionChild<OptionNumber<std::uint32_t>> retries{main_config.get_retries_option()};
#pragma GCC diagnostic pop
    OptionChild<OptionNumber<std::uint32_t>> bandwidth{main_config.get_bandwidth_option()};
    OptionChild<OptionNumber<std::uint32_t>> minrate{main_config.get_minrate_option()};
    OptionChild<OptionEnum> ip_resolve{main_config.get_ip_resolve_option()};
    OptionChild<OptionNumber<float>> throttle{main_config.get_throttle_option()};
    OptionChild<OptionSeconds> timeout{main_config.get_timeout_option()};
    OptionChild<OptionNumber<std::uint32_t>> max_parallel_downloads{main_config.get_max_parallel_downloads_option()};
    OptionChild<OptionNumber<std::uint32_t>> max_downloads_per_mirror{
        main_config.get_max_downloads_per_mirror_option()};
    OptionChild<OptionSeconds> metadata_expire{main_config.get_metadata_expire_option()};
    OptionNumber<std::int32_t> cost{1000};
    OptionNumber<std::int32_t> priority{99};
    OptionBool module_hotfixes{false};
    OptionChild<OptionString> sslcacert{main_config.get_sslcacert_option()};
    OptionChild<OptionBool> sslverify{main_config.get_sslverify_option()};
    OptionChild<OptionString> sslclientcert{main_config.get_sslclientcert_option()};
    OptionChild<OptionString> sslclientkey{main_config.get_sslclientkey_option()};
    OptionChild<OptionString> proxy_sslcacert{main_config.get_proxy_sslcacert_option()};
    OptionChild<OptionBool> proxy_sslverify{main_config.get_proxy_sslverify_option()};
    OptionChild<OptionString> proxy_sslclientcert{main_config.get_proxy_sslclientcert_option()};
    OptionChild<OptionString> proxy_sslclientkey{main_config.get_proxy_sslclientkey_option()};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    OptionChild<OptionBool> deltarpm{main_config.get_deltarpm_option()};
    OptionChild<OptionNumber<std::uint32_t>> deltarpm_percentage{main_config.get_deltarpm_percentage_option()};
#pragma GCC diagnostic pop
    OptionChild<OptionBool> skip_if_unavailable{main_config.get_skip_if_unavailable_option()};
    OptionString enabled_metadata{""};
    OptionChild<OptionString> user_agent{main_config.get_user_agent_option()};
    OptionChild<OptionBool> countme{main_config.get_countme_option()};
    OptionEnum failovermethod{"priority", {"priority", "roundrobin"}};
    OptionChild<OptionBool> build_cache{main_config.get_build_cache_option()};
};

ConfigRepo::Impl::Impl(Config & owner, ConfigMain & main_config, const std::string & id)
    : main_config(main_config),
      id(id) {
    owner.opt_binds().add("name", name);
    owner.opt_binds().add("enabled", enabled);
    owner.opt_binds().add("cachedir", basecachedir);
    owner.opt_binds().add("baseurl", baseurl);
    owner.opt_binds().add("mirrorlist", mirrorlist);
    owner.opt_binds().add("metalink", metalink);
    owner.opt_binds().add("type", type);
    owner.opt_binds().add("mediaid", mediaid);
    owner.opt_binds().add("gpgkey", gpgkey);
    owner.opt_binds().add("excludepkgs", excludepkgs);
    owner.opt_binds().add("exclude", excludepkgs);  //compatibility with yum
    owner.opt_binds().add("includepkgs", includepkgs);
    owner.opt_binds().add("fastestmirror", fastestmirror);

    owner.opt_binds().add(
        "proxy",
        proxy,
        [&](Option::Priority priority, const std::string & value) {
            auto tmp_value(value);
            for (auto & ch : tmp_value) {
                ch = static_cast<char>(std::tolower(ch));
            }
            if (tmp_value == "_none_") {
                proxy.set(priority, "");
            } else {
                proxy.set(priority, value);
            }
        },
        nullptr,
        false);

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

    owner.opt_binds().add("username", username);
    owner.opt_binds().add("password", password);
    owner.opt_binds().add("protected_packages", protected_packages);
    owner.opt_binds().add("pkg_gpgcheck", pkg_gpgcheck);
    // Compatibility alias for pkg_gpgcheck
    owner.opt_binds().add("gpgcheck", pkg_gpgcheck);
    owner.opt_binds().add("repo_gpgcheck", repo_gpgcheck);
    owner.opt_binds().add("enablegroups", enablegroups);
    owner.opt_binds().add("retries", retries);
    owner.opt_binds().add("bandwidth", bandwidth);
    owner.opt_binds().add("minrate", minrate);
    owner.opt_binds().add("ip_resolve", ip_resolve);
    owner.opt_binds().add("throttle", throttle);
    owner.opt_binds().add("timeout", timeout);
    owner.opt_binds().add("max_parallel_downloads", max_parallel_downloads);
    owner.opt_binds().add("max_downloads_per_mirror", max_downloads_per_mirror);
    owner.opt_binds().add("metadata_expire", metadata_expire);
    owner.opt_binds().add("cost", cost);
    owner.opt_binds().add("priority", priority);
    owner.opt_binds().add("module_hotfixes", module_hotfixes);
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
    owner.opt_binds().add("enabled_metadata", enabled_metadata);
    owner.opt_binds().add("user_agent", user_agent);
    owner.opt_binds().add("countme", countme);
    owner.opt_binds().add("build_cache", build_cache);
}

ConfigRepo::ConfigRepo(ConfigMain & main_config, const std::string & id) : p_impl(new Impl(*this, main_config, id)) {}
ConfigRepo::~ConfigRepo() = default;
ConfigRepo::ConfigRepo(ConfigRepo && src) : p_impl(std::move(src.p_impl)) {}

ConfigMain & ConfigRepo::get_main_config() {
    return p_impl->main_config;
}
const ConfigMain & ConfigRepo::get_main_config() const {
    return p_impl->main_config;
}

std::string ConfigRepo::get_id() const {
    return p_impl->id;
}

OptionString & ConfigRepo::get_name_option() {
    return p_impl->name;
}
const OptionString & ConfigRepo::get_name_option() const {
    return p_impl->name;
}
OptionBool & ConfigRepo::get_enabled_option() {
    return p_impl->enabled;
}
const OptionBool & ConfigRepo::get_enabled_option() const {
    return p_impl->enabled;
}

OptionChild<OptionString> & ConfigRepo::get_basecachedir_option() {
    return p_impl->basecachedir;
}
const OptionChild<OptionString> & ConfigRepo::get_basecachedir_option() const {
    return p_impl->basecachedir;
}

OptionStringList & ConfigRepo::get_baseurl_option() {
    return p_impl->baseurl;
}
const OptionStringList & ConfigRepo::get_baseurl_option() const {
    return p_impl->baseurl;
}

OptionString & ConfigRepo::get_mirrorlist_option() {
    return p_impl->mirrorlist;
}
const OptionString & ConfigRepo::get_mirrorlist_option() const {
    return p_impl->mirrorlist;
}

OptionString & ConfigRepo::get_metalink_option() {
    return p_impl->metalink;
}
const OptionString & ConfigRepo::get_metalink_option() const {
    return p_impl->metalink;
}

OptionString & ConfigRepo::get_type_option() {
    return p_impl->type;
}
const OptionString & ConfigRepo::get_type_option() const {
    return p_impl->type;
}

OptionString & ConfigRepo::get_mediaid_option() {
    return p_impl->mediaid;
}
const OptionString & ConfigRepo::get_mediaid_option() const {
    return p_impl->mediaid;
}

OptionStringList & ConfigRepo::get_gpgkey_option() {
    return p_impl->gpgkey;
}
const OptionStringList & ConfigRepo::get_gpgkey_option() const {
    return p_impl->gpgkey;
}

OptionStringAppendList & ConfigRepo::get_excludepkgs_option() {
    return p_impl->excludepkgs;
}
const OptionStringAppendList & ConfigRepo::get_excludepkgs_option() const {
    return p_impl->excludepkgs;
}

OptionStringAppendList & ConfigRepo::get_includepkgs_option() {
    return p_impl->includepkgs;
}
const OptionStringAppendList & ConfigRepo::get_includepkgs_option() const {
    return p_impl->includepkgs;
}

OptionChild<OptionBool> & ConfigRepo::get_fastestmirror_option() {
    return p_impl->fastestmirror;
}
const OptionChild<OptionBool> & ConfigRepo::get_fastestmirror_option() const {
    return p_impl->fastestmirror;
}

OptionChild<OptionString> & ConfigRepo::get_proxy_option() {
    return p_impl->proxy;
}
const OptionChild<OptionString> & ConfigRepo::get_proxy_option() const {
    return p_impl->proxy;
}

OptionChild<OptionString> & ConfigRepo::get_proxy_username_option() {
    return p_impl->proxy_username;
}
const OptionChild<OptionString> & ConfigRepo::get_proxy_username_option() const {
    return p_impl->proxy_username;
}

OptionChild<OptionString> & ConfigRepo::get_proxy_password_option() {
    return p_impl->proxy_password;
}
const OptionChild<OptionString> & ConfigRepo::get_proxy_password_option() const {
    return p_impl->proxy_password;
}

OptionChild<OptionStringSet> & ConfigRepo::get_proxy_auth_method_option() {
    return p_impl->proxy_auth_method;
}
const OptionChild<OptionStringSet> & ConfigRepo::get_proxy_auth_method_option() const {
    return p_impl->proxy_auth_method;
}

OptionChild<OptionString> & ConfigRepo::get_username_option() {
    return p_impl->username;
}
const OptionChild<OptionString> & ConfigRepo::get_username_option() const {
    return p_impl->username;
}

OptionChild<OptionString> & ConfigRepo::get_password_option() {
    return p_impl->password;
}
const OptionChild<OptionString> & ConfigRepo::get_password_option() const {
    return p_impl->password;
}

OptionChild<OptionStringAppendList> & ConfigRepo::get_protected_packages_option() {
    return p_impl->protected_packages;
}
const OptionChild<OptionStringAppendList> & ConfigRepo::get_protected_packages_option() const {
    return p_impl->protected_packages;
}

OptionChild<OptionBool> & ConfigRepo::get_gpgcheck_option() {
    LIBDNF5_DEPRECATED("Use get_pkg_gpgcheck_option()");
    return p_impl->pkg_gpgcheck;
}
const OptionChild<OptionBool> & ConfigRepo::get_gpgcheck_option() const {
    LIBDNF5_DEPRECATED("Use get_pkg_gpgcheck_option() const");
    return p_impl->pkg_gpgcheck;
}

OptionChild<OptionBool> & ConfigRepo::get_pkg_gpgcheck_option() {
    return p_impl->pkg_gpgcheck;
}
const OptionChild<OptionBool> & ConfigRepo::get_pkg_gpgcheck_option() const {
    return p_impl->pkg_gpgcheck;
}

OptionChild<OptionBool> & ConfigRepo::get_repo_gpgcheck_option() {
    return p_impl->repo_gpgcheck;
}
const OptionChild<OptionBool> & ConfigRepo::get_repo_gpgcheck_option() const {
    return p_impl->repo_gpgcheck;
}

OptionChild<OptionBool> & ConfigRepo::get_enablegroups_option() {
    return p_impl->enablegroups;
}
const OptionChild<OptionBool> & ConfigRepo::get_enablegroups_option() const {
    return p_impl->enablegroups;
}

OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_retries_option() {
    LIBDNF5_DEPRECATED("The option does nothing");
    return p_impl->retries;
}
const OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_retries_option() const {
    LIBDNF5_DEPRECATED("The option does nothing");
    return p_impl->retries;
}

OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_bandwidth_option() {
    return p_impl->bandwidth;
}
const OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_bandwidth_option() const {
    return p_impl->bandwidth;
}

OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_minrate_option() {
    return p_impl->minrate;
}
const OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_minrate_option() const {
    return p_impl->minrate;
}

OptionChild<OptionEnum> & ConfigRepo::get_ip_resolve_option() {
    return p_impl->ip_resolve;
}
const OptionChild<OptionEnum> & ConfigRepo::get_ip_resolve_option() const {
    return p_impl->ip_resolve;
}

OptionChild<OptionNumber<float>> & ConfigRepo::get_throttle_option() {
    return p_impl->throttle;
}
const OptionChild<OptionNumber<float>> & ConfigRepo::get_throttle_option() const {
    return p_impl->throttle;
}

OptionChild<OptionSeconds> & ConfigRepo::get_timeout_option() {
    return p_impl->timeout;
}
const OptionChild<OptionSeconds> & ConfigRepo::get_timeout_option() const {
    return p_impl->timeout;
}

OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_max_parallel_downloads_option() {
    return p_impl->max_parallel_downloads;
}
const OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_max_parallel_downloads_option() const {
    return p_impl->max_parallel_downloads;
}

OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_max_downloads_per_mirror_option() {
    return p_impl->max_downloads_per_mirror;
}
const OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_max_downloads_per_mirror_option() const {
    return p_impl->max_downloads_per_mirror;
}

OptionChild<OptionSeconds> & ConfigRepo::get_metadata_expire_option() {
    return p_impl->metadata_expire;
}
const OptionChild<OptionSeconds> & ConfigRepo::get_metadata_expire_option() const {
    return p_impl->metadata_expire;
}

OptionNumber<std::int32_t> & ConfigRepo::get_cost_option() {
    return p_impl->cost;
}
const OptionNumber<std::int32_t> & ConfigRepo::get_cost_option() const {
    return p_impl->cost;
}

OptionNumber<std::int32_t> & ConfigRepo::get_priority_option() {
    return p_impl->priority;
}
const OptionNumber<std::int32_t> & ConfigRepo::get_priority_option() const {
    return p_impl->priority;
}

OptionBool & ConfigRepo::get_module_hotfixes_option() {
    return p_impl->module_hotfixes;
}
const OptionBool & ConfigRepo::get_module_hotfixes_option() const {
    return p_impl->module_hotfixes;
}

OptionChild<OptionString> & ConfigRepo::get_sslcacert_option() {
    return p_impl->sslcacert;
}
const OptionChild<OptionString> & ConfigRepo::get_sslcacert_option() const {
    return p_impl->sslcacert;
}

OptionChild<OptionBool> & ConfigRepo::get_sslverify_option() {
    return p_impl->sslverify;
}
const OptionChild<OptionBool> & ConfigRepo::get_sslverify_option() const {
    return p_impl->sslverify;
}

OptionChild<OptionString> & ConfigRepo::get_sslclientcert_option() {
    return p_impl->sslclientcert;
}
const OptionChild<OptionString> & ConfigRepo::get_sslclientcert_option() const {
    return p_impl->sslclientcert;
}

OptionChild<OptionString> & ConfigRepo::get_sslclientkey_option() {
    return p_impl->sslclientkey;
}
const OptionChild<OptionString> & ConfigRepo::get_sslclientkey_option() const {
    return p_impl->sslclientkey;
}

OptionChild<OptionString> & ConfigRepo::get_proxy_sslcacert_option() {
    return p_impl->proxy_sslcacert;
}

const OptionChild<OptionString> & ConfigRepo::get_proxy_sslcacert_option() const {
    return p_impl->proxy_sslcacert;
}

OptionChild<OptionBool> & ConfigRepo::get_proxy_sslverify_option() {
    return p_impl->proxy_sslverify;
}

const OptionChild<OptionBool> & ConfigRepo::get_proxy_sslverify_option() const {
    return p_impl->proxy_sslverify;
}

OptionChild<OptionString> & ConfigRepo::get_proxy_sslclientcert_option() {
    return p_impl->proxy_sslclientcert;
}

const OptionChild<OptionString> & ConfigRepo::get_proxy_sslclientcert_option() const {
    return p_impl->proxy_sslclientcert;
}

OptionChild<OptionString> & ConfigRepo::get_proxy_sslclientkey_option() {
    return p_impl->proxy_sslclientkey;
}

const OptionChild<OptionString> & ConfigRepo::get_proxy_sslclientkey_option() const {
    return p_impl->proxy_sslclientkey;
}

OptionChild<OptionBool> & ConfigRepo::get_deltarpm_option() {
    LIBDNF5_DEPRECATED("The option does nothing");
    return p_impl->deltarpm;
}
const OptionChild<OptionBool> & ConfigRepo::get_deltarpm_option() const {
    LIBDNF5_DEPRECATED("The option does nothing");
    return p_impl->deltarpm;
}

OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_deltarpm_percentage_option() {
    LIBDNF5_DEPRECATED("The option does nothing");
    return p_impl->deltarpm_percentage;
}
const OptionChild<OptionNumber<std::uint32_t>> & ConfigRepo::get_deltarpm_percentage_option() const {
    LIBDNF5_DEPRECATED("The option does nothing");
    return p_impl->deltarpm_percentage;
}

OptionChild<OptionBool> & ConfigRepo::get_skip_if_unavailable_option() {
    return p_impl->skip_if_unavailable;
}
const OptionChild<OptionBool> & ConfigRepo::get_skip_if_unavailable_option() const {
    return p_impl->skip_if_unavailable;
}

OptionString & ConfigRepo::get_enabled_metadata_option() {
    return p_impl->enabled_metadata;
}
const OptionString & ConfigRepo::get_enabled_metadata_option() const {
    return p_impl->enabled_metadata;
}

OptionChild<OptionString> & ConfigRepo::get_user_agent_option() {
    return p_impl->user_agent;
}
const OptionChild<OptionString> & ConfigRepo::get_user_agent_option() const {
    return p_impl->user_agent;
}

OptionChild<OptionBool> & ConfigRepo::get_countme_option() {
    return p_impl->countme;
}
const OptionChild<OptionBool> & ConfigRepo::get_countme_option() const {
    return p_impl->countme;
}

OptionEnum & ConfigRepo::get_failovermethod_option() {
    return p_impl->failovermethod;
}
const OptionEnum & ConfigRepo::get_failovermethod_option() const {
    return p_impl->failovermethod;
}

OptionChild<OptionBool> & ConfigRepo::get_build_cache_option() {
    return p_impl->build_cache;
}
const OptionChild<OptionBool> & ConfigRepo::get_build_cache_option() const {
    return p_impl->build_cache;
}


std::string ConfigRepo::get_unique_id() const {
    std::string tmp;
    if (get_metalink_option().empty() || (tmp = get_metalink_option().get_value()).empty()) {
        if (get_mirrorlist_option().empty() || (tmp = get_mirrorlist_option().get_value()).empty()) {
            if (!get_baseurl_option().get_value().empty())
                tmp = get_baseurl_option().get_value()[0];
            if (tmp.empty())
                tmp = p_impl->id;
        }
    }

    auto chksum_obj = solv_chksum_create(REPOKEY_TYPE_SHA256);
    solv_chksum_add(chksum_obj, tmp.c_str(), static_cast<int>(tmp.length()));
    int chksum_len;
    auto chksum = solv_chksum_get(chksum_obj, &chksum_len);
    static constexpr int USE_CHECKSUM_BYTES = 8;
    if (chksum_len < USE_CHECKSUM_BYTES) {
        solv_chksum_free(chksum_obj, nullptr);
        throw RuntimeError(M_("get_unique_id(): Computation of SHA256 failed"));
    }
    char chksum_cstr[USE_CHECKSUM_BYTES * 2 + 1];
    solv_bin2hex(chksum, USE_CHECKSUM_BYTES, chksum_cstr);
    solv_chksum_free(chksum_obj, nullptr);

    return p_impl->id + "-" + chksum_cstr;
}


std::string ConfigRepo::get_cachedir() const {
    std::filesystem::path repo_dir{get_basecachedir_option().get_value()};
    return repo_dir / get_unique_id();
}


std::string ConfigRepo::get_persistdir() const {
    std::filesystem::path main_persistdir{get_main_config().get_persistdir_option().get_value()};
    return main_persistdir / "repos" / get_unique_id();
}

void ConfigRepo::load_from_parser(
    const ConfigParser & parser,
    const std::string & section,
    const Vars & vars,
    Logger & logger,
    Option::Priority priority) {
    Config::load_from_parser(parser, section, vars, logger, priority);
}

}  // namespace libdnf5::repo
