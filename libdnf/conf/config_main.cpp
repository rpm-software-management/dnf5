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

#include "ConfigMain.hpp"
#include "Const.hpp"
#include "Config-private.hpp"
#include "libdnf/utils/os-release.hpp"
#include "utils.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <istream>
#include <ostream>
#include <fstream>
#include <utility>

#include <dirent.h>
#include <glob.h>
#include <string.h>
#include <sys/types.h>

#include "bgettext/bgettext-lib.h"
#include "tinyformat/tinyformat.hpp"

extern char **environ;

namespace libdnf {

/**
* @brief Converts a friendly bandwidth option to bytes
*
* Function converts a friendly bandwidth option to bytes.  The input
* should be a string containing a (possibly floating point)
* number followed by an optional single character unit. Valid
* units are 'k', 'M', 'G'. Case is ignored. The convention that
* 1k = 1024 bytes is used.
*
* @param str Bandwidth as user friendly string
* @return int Number of bytes
*/
static int strToBytes(const std::string & str)
{
    if (str.empty())
        throw Option::InvalidValue(_("no value specified"));

    std::size_t idx;
    auto res = std::stod(str, &idx);
    if (res < 0)
        throw Option::InvalidValue(tfm::format(_("seconds value '%s' must not be negative"), str));

    if (idx < str.length()) {
        if (idx < str.length() - 1)
            throw Option::InvalidValue(tfm::format(_("could not convert '%s' to bytes"), str));
        switch (str.back()) {
            case 'k': case 'K':
                res *= 1024;
                break;
            case 'm': case 'M':
                res *= 1024 * 1024;
                break;
            case 'g': case 'G':
                res *= 1024 * 1024 * 1024;
                break;
            default:
                throw Option::InvalidValue(tfm::format(_("unknown unit '%s'"), str.back()));
        }
    }

    return res;
}

static void addFromFile(std::ostream & out, const std::string & filePath)
{
    std::ifstream ifs(filePath);
    if (!ifs)
        throw std::runtime_error("addFromFile(): Can't open file");
    ifs.exceptions(std::ifstream::badbit);

    std::string line;
    while (!ifs.eof()) {
        std::getline(ifs, line);
        auto start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos)
            continue;
        if (line[start] == '#')
            continue;
        auto end = line.find_last_not_of(" \t\r");

        out.write(line.c_str()+start, end - start + 1);
        out.put(' ');
    }
}

static void addFromFiles(std::ostream & out, const std::string & globPath)
{
    glob_t globBuf;
    glob(globPath.c_str(), GLOB_MARK | GLOB_NOSORT, NULL, &globBuf);
    for (size_t i = 0; i < globBuf.gl_pathc; ++i) {
        auto path = globBuf.gl_pathv[i];
        if (path[strlen(path)-1] != '/')
            addFromFile(out, path);
    }
    globfree(&globBuf);
}

/**
* @brief Replaces globs (like /etc/foo.d/\\*.foo) by content of matching files.
*
* Ignores comment lines (start with '#') and blank lines in files.
* Result:
* Words delimited by spaces. Characters ',' and '\n' are replaced by spaces.
* Extra spaces are removed.
* @param strWithGlobs Input string with globs
* @return Words delimited by space
*/
static std::string resolveGlobs(const std::string & strWithGlobs)
{
    std::ostringstream res;
    std::string::size_type start{0};
    while (start < strWithGlobs.length()) {
        auto end = strWithGlobs.find_first_of(" ,\n", start);
        if (strWithGlobs.compare(start, 5, "glob:") == 0) {
            start += 5;
            if (start >= strWithGlobs.length())
                break;
            if (end == std::string::npos) {
                addFromFiles(res, strWithGlobs.substr(start));
                break;
            }
            if (end - start != 0)
                addFromFiles(res, strWithGlobs.substr(start, end - start));
        } else {
            if (end == std::string::npos) {
                res << strWithGlobs.substr(start);
                break;
            }
            if (end - start != 0)
                res << strWithGlobs.substr(start, end - start) << " ";
        }
        start = end + 1;
    }
    return res.str();
}

class ConfigMain::Impl {
    friend class ConfigMain;

    Impl(Config & owner);

    Config & owner;

    OptionNumber<std::int32_t> debuglevel{2, 0, 10};
    OptionNumber<std::int32_t> errorlevel{3, 0, 10};
    OptionPath installroot{"/", false, true};
    OptionPath config_file_path{CONF_FILENAME};
    OptionBool plugins{true};
    OptionStringList pluginpath{std::vector<std::string>{}};
    OptionStringList pluginconfpath{std::vector<std::string>{}};
    OptionPath persistdir{PERSISTDIR};
    OptionBool transformdb{true};
    OptionNumber<std::int32_t> recent{7, 0};
    OptionBool reset_nice{true};
    OptionPath system_cachedir{SYSTEM_CACHEDIR};
    OptionBool cacheonly{false};
    OptionBool keepcache{false};
    OptionString logdir{"/var/log"};
    OptionNumber<std::int32_t> log_size{1024 * 1024, strToBytes};
    OptionNumber<std::int32_t> log_rotate{4, 0};
    OptionStringList varsdir{VARS_DIRS};
    OptionStringList reposdir{{"/etc/yum.repos.d", "/etc/yum/repos.d", "/etc/distro.repos.d"}};
    OptionBool debug_solver{false};
    OptionStringList installonlypkgs{INSTALLONLYPKGS};
    OptionStringList group_package_types{GROUP_PACKAGE_TYPES};

    OptionNumber<std::uint32_t> installonly_limit{3, 0,
        [](const std::string & value)->std::uint32_t{
            if (value == "<off>")
                return 0;
            try {
                return std::stoul(value);
            }
            catch (...) {
                return 0;
            }
        }
    };

    OptionStringList tsflags{std::vector<std::string>{}};
    OptionBool assumeyes{false};
    OptionBool assumeno{false};
    OptionBool check_config_file_age{true};
    OptionBool defaultyes{false};
    OptionBool diskspacecheck{true};
    OptionBool localpkg_gpgcheck{false};
    OptionBool gpgkey_dns_verification{false};
    OptionBool obsoletes{true};
    OptionBool showdupesfromrepos{false};
    OptionBool exit_on_lock{false};
    OptionSeconds metadata_timer_sync{60 * 60 * 3}; // 3 hours
    OptionStringList disable_excludes{std::vector<std::string>{}};
    OptionEnum<std::string> multilib_policy{"best", {"best", "all"}}; // :api
    OptionBool best{false}; // :api
    OptionBool install_weak_deps{true};
    OptionString bugtracker_url{BUGTRACKER};
    OptionBool zchunk{true};

    OptionEnum<std::string> color{"auto", {"auto", "never", "always"},
        [](const std::string & value){
            const std::array<const char *, 4> always{{"on", "yes", "1", "true"}};
            const std::array<const char *, 4> never{{"off", "no", "0", "false"}};
            const std::array<const char *, 2> aut{{"tty", "if-tty"}};
            std::string tmp;
            if (std::find(always.begin(), always.end(), value) != always.end())
                tmp = "always";
            else if (std::find(never.begin(), never.end(), value) != never.end())
                tmp = "never";
            else if (std::find(aut.begin(), aut.end(), value) != aut.end())
                tmp = "auto";
            else
                tmp = value;
            return tmp;
        }
    };

    OptionString color_list_installed_older{"yellow"};
    OptionString color_list_installed_newer{"bold,yellow"};
    OptionString color_list_installed_reinstall{"dim,cyan"};
    OptionString color_list_installed_extra{"bold,red"};
    OptionString color_list_available_upgrade{"bold,blue"};
    OptionString color_list_available_downgrade{"dim,magenta"};
    OptionString color_list_available_reinstall{"bold,underline,green"};
    OptionString color_list_available_install{"bold,cyan"};
    OptionString color_update_installed{"dim,red"};
    OptionString color_update_local{"dim,green"};
    OptionString color_update_remote{"bold,green"};
    OptionString color_search_match{"bold,magenta"};
    OptionBool history_record{true};
    OptionStringList history_record_packages{std::vector<std::string>{"dnf", "rpm"}};
    OptionString rpmverbosity{"info"};
    OptionBool strict{true}; // :api
    OptionBool skip_broken{false}; // :yum-compatibility
    OptionBool autocheck_running_kernel{true}; // :yum-compatibility
    OptionBool clean_requirements_on_remove{true};

    OptionEnum<std::string> history_list_view{"commands", {"single-user-commands", "users", "commands"},
        [](const std::string & value){
            if (value == "cmds" || value == "default")
                return std::string("commands");
            else
                return value;
        }
    };

    OptionBool upgrade_group_objects_upgrade{true}; // :api
    OptionPath destdir{nullptr};
    OptionString comment{nullptr};
    OptionBool downloadonly{false}; // runtime only option
    OptionBool ignorearch{false};
    OptionString module_platform_id{nullptr};

    OptionString user_agent{getUserAgent()};
    OptionBool countme{false};

    // Repo main config

    OptionNumber<std::uint32_t> retries{10};
    OptionString cachedir{nullptr};
    OptionBool fastestmirror{false};
    OptionStringList excludepkgs{std::vector<std::string>{}};
    OptionStringList includepkgs{std::vector<std::string>{}};
    OptionString proxy{""};
    OptionString proxy_username{nullptr};
    OptionString proxy_password{nullptr};

    OptionEnum<std::string> proxy_auth_method{"any", {"any", "none", "basic", "digest",
        "negotiate", "ntlm", "digest_ie", "ntlm_wb"},
        [](const std::string & value){
            auto tmp = value;
            std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
            return tmp;
        }
    };

    OptionStringList protected_packages{resolveGlobs("dnf glob:/etc/yum/protected.d/*.conf " \
                                          "glob:/etc/dnf/protected.d/*.conf")};
    OptionString username{""};
    OptionString password{""};
    OptionBool gpgcheck{false};
    OptionBool repo_gpgcheck{false};
    OptionBool enabled{true};
    OptionBool enablegroups{true};
    OptionNumber<std::uint32_t> bandwidth{0, strToBytes};
    OptionNumber<std::uint32_t> minrate{1000, strToBytes};

    OptionEnum<std::string> ip_resolve{"whatever", {"ipv4", "ipv6", "whatever"},
        [](const std::string & value){
            auto tmp = value;
            if (value == "4") tmp = "ipv4";
            else if (value == "6") tmp = "ipv6";
            else std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
            return tmp;
        }
    };

    OptionNumber<float> throttle{0, 0,
        [](const std::string & value)->float{
            if (!value.empty() && value.back()=='%') {
                std::size_t idx;
                auto res = std::stod(value, &idx);
                if (res < 0 || res > 100)
                    throw Option::InvalidValue(tfm::format(_("percentage '%s' is out of range"), value));
                return res/100;
            }
            return strToBytes(value);
        }
    };

    OptionSeconds timeout{30};
    OptionNumber<std::uint32_t> max_parallel_downloads{3, 1};
    OptionSeconds metadata_expire{60 * 60 * 48};
    OptionString sslcacert{""};
    OptionBool sslverify{true};
    OptionString sslclientcert{""};
    OptionString sslclientkey{""};
    OptionBool deltarpm{true};
    OptionNumber<std::uint32_t> deltarpm_percentage{75};
    OptionBool skip_if_unavailable{false};
};

ConfigMain::Impl::Impl(Config & owner)
: owner(owner)
{
    owner.optBinds().add("debuglevel", debuglevel);
    owner.optBinds().add("errorlevel", errorlevel);
    owner.optBinds().add("installroot", installroot);
    owner.optBinds().add("config_file_path", config_file_path);
    owner.optBinds().add("plugins", plugins);
    owner.optBinds().add("pluginpath", pluginpath);
    owner.optBinds().add("pluginconfpath", pluginconfpath);
    owner.optBinds().add("persistdir", persistdir);
    owner.optBinds().add("transformdb", transformdb);
    owner.optBinds().add("recent", recent);
    owner.optBinds().add("reset_nice", reset_nice);
    owner.optBinds().add("system_cachedir", system_cachedir);
    owner.optBinds().add("cacheonly", cacheonly);
    owner.optBinds().add("keepcache", keepcache);
    owner.optBinds().add("logdir", logdir);
    owner.optBinds().add("log_size", log_size);
    owner.optBinds().add("log_rotate", log_rotate);
    owner.optBinds().add("varsdir", varsdir);
    owner.optBinds().add("reposdir", reposdir);
    owner.optBinds().add("debug_solver", debug_solver);

    owner.optBinds().add("installonlypkgs", installonlypkgs,
        [&](Option::Priority priority, const std::string & value){
            optionTListAppend(installonlypkgs, priority, value);
        }, nullptr, true
    );

    owner.optBinds().add("group_package_types", group_package_types);
    owner.optBinds().add("installonly_limit", installonly_limit);

    owner.optBinds().add("tsflags", tsflags,
        [&](Option::Priority priority, const std::string & value){
            optionTListAppend(tsflags, priority, value);
        }, nullptr, true
    );

    owner.optBinds().add("assumeyes", assumeyes);
    owner.optBinds().add("assumeno", assumeno);
    owner.optBinds().add("check_config_file_age", check_config_file_age);
    owner.optBinds().add("defaultyes", defaultyes);
    owner.optBinds().add("diskspacecheck", diskspacecheck);
    owner.optBinds().add("localpkg_gpgcheck", localpkg_gpgcheck);
    owner.optBinds().add("gpgkey_dns_verification", gpgkey_dns_verification);
    owner.optBinds().add("obsoletes", obsoletes);
    owner.optBinds().add("showdupesfromrepos", showdupesfromrepos);
    owner.optBinds().add("exit_on_lock", exit_on_lock);
    owner.optBinds().add("metadata_timer_sync", metadata_timer_sync);
    owner.optBinds().add("disable_excludes", disable_excludes);
    owner.optBinds().add("multilib_policy", multilib_policy);
    owner.optBinds().add("best", best);
    owner.optBinds().add("install_weak_deps", install_weak_deps);
    owner.optBinds().add("bugtracker_url", bugtracker_url);
    owner.optBinds().add("zchunk", zchunk);
    owner.optBinds().add("color", color);
    owner.optBinds().add("color_list_installed_older", color_list_installed_older);
    owner.optBinds().add("color_list_installed_newer", color_list_installed_newer);
    owner.optBinds().add("color_list_installed_reinstall", color_list_installed_reinstall);
    owner.optBinds().add("color_list_installed_extra", color_list_installed_extra);
    owner.optBinds().add("color_list_available_upgrade", color_list_available_upgrade);
    owner.optBinds().add("color_list_available_downgrade", color_list_available_downgrade);
    owner.optBinds().add("color_list_available_reinstall", color_list_available_reinstall);
    owner.optBinds().add("color_list_available_install", color_list_available_install);
    owner.optBinds().add("color_update_installed", color_update_installed);
    owner.optBinds().add("color_update_local", color_update_local);
    owner.optBinds().add("color_update_remote", color_update_remote);
    owner.optBinds().add("color_search_match", color_search_match);
    owner.optBinds().add("history_record", history_record);
    owner.optBinds().add("history_record_packages", history_record_packages);
    owner.optBinds().add("rpmverbosity", rpmverbosity);
    owner.optBinds().add("strict", strict);
    owner.optBinds().add("skip_broken", skip_broken);
    owner.optBinds().add("autocheck_running_kernel", autocheck_running_kernel);
    owner.optBinds().add("clean_requirements_on_remove", clean_requirements_on_remove);
    owner.optBinds().add("history_list_view", history_list_view);
    owner.optBinds().add("upgrade_group_objects_upgrade", upgrade_group_objects_upgrade);
    owner.optBinds().add("destdir", destdir);
    owner.optBinds().add("comment", comment);
    owner.optBinds().add("ignorearch", ignorearch);
    owner.optBinds().add("module_platform_id", module_platform_id);
    owner.optBinds().add("user_agent", user_agent);
    owner.optBinds().add("countme", countme);

    // Repo main config

    owner.optBinds().add("retries", retries);
    owner.optBinds().add("cachedir", cachedir);
    owner.optBinds().add("fastestmirror", fastestmirror);

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

    owner.optBinds().add("proxy", proxy);
    owner.optBinds().add("proxy_username", proxy_username);
    owner.optBinds().add("proxy_password", proxy_password);
    owner.optBinds().add("proxy_auth_method", proxy_auth_method);
    owner.optBinds().add("protected_packages", protected_packages,
        [&](Option::Priority priority, const std::string & value){
            if (priority >= protected_packages.getPriority())
                protected_packages.set(priority, resolveGlobs(value));
        }, nullptr, false
    );

    owner.optBinds().add("username", username);
    owner.optBinds().add("password", password);
    owner.optBinds().add("gpgcheck", gpgcheck);
    owner.optBinds().add("repo_gpgcheck", repo_gpgcheck);
    owner.optBinds().add("enabled", enabled);
    owner.optBinds().add("enablegroups", enablegroups);
    owner.optBinds().add("bandwidth", bandwidth);
    owner.optBinds().add("minrate", minrate);
    owner.optBinds().add("ip_resolve", ip_resolve);
    owner.optBinds().add("throttle", throttle);
    owner.optBinds().add("timeout", timeout);
    owner.optBinds().add("max_parallel_downloads", max_parallel_downloads);
    owner.optBinds().add("metadata_expire", metadata_expire);
    owner.optBinds().add("sslcacert", sslcacert);
    owner.optBinds().add("sslverify", sslverify);
    owner.optBinds().add("sslclientcert", sslclientcert);
    owner.optBinds().add("sslclientkey", sslclientkey);
    owner.optBinds().add("deltarpm", deltarpm);
    owner.optBinds().add("deltarpm_percentage", deltarpm_percentage);
    owner.optBinds().add("skip_if_unavailable", skip_if_unavailable);
}

ConfigMain::ConfigMain() { pImpl = std::unique_ptr<Impl>(new Impl(*this)); }
ConfigMain::~ConfigMain() = default;

OptionNumber<std::int32_t> & ConfigMain::debuglevel() { return pImpl->debuglevel; }
OptionNumber<std::int32_t> & ConfigMain::errorlevel() { return pImpl->errorlevel; }
OptionString & ConfigMain::installroot() { return pImpl->installroot; }
OptionString & ConfigMain::config_file_path() { return pImpl->config_file_path; }
OptionBool & ConfigMain::plugins() { return pImpl->plugins; }
OptionStringList & ConfigMain::pluginpath() { return pImpl->pluginpath; }
OptionStringList & ConfigMain::pluginconfpath() { return pImpl->pluginconfpath; }
OptionString & ConfigMain::persistdir() { return pImpl->persistdir; }
OptionBool & ConfigMain::transformdb() { return pImpl->transformdb; }
OptionNumber<std::int32_t> & ConfigMain::recent() { return pImpl->recent; }
OptionBool & ConfigMain::reset_nice() { return pImpl->reset_nice; }
OptionString & ConfigMain::system_cachedir() { return pImpl->system_cachedir; }
OptionBool & ConfigMain::cacheonly() { return pImpl->cacheonly; }
OptionBool & ConfigMain::keepcache() { return pImpl->keepcache; }
OptionString & ConfigMain::logdir() { return pImpl->logdir; }
OptionNumber<std::int32_t> & ConfigMain::log_size() { return pImpl->log_size; }
OptionNumber<std::int32_t> & ConfigMain::log_rotate() { return pImpl->log_rotate; }
OptionStringList & ConfigMain::varsdir() { return pImpl->varsdir; }
OptionStringList & ConfigMain::reposdir() { return pImpl->reposdir; }
OptionBool & ConfigMain::debug_solver() { return pImpl->debug_solver; }
OptionStringList & ConfigMain::installonlypkgs() { return pImpl->installonlypkgs; }
OptionStringList & ConfigMain::group_package_types() { return pImpl->group_package_types; }
OptionNumber<std::uint32_t> & ConfigMain::installonly_limit() { return pImpl->installonly_limit; }
OptionStringList & ConfigMain::tsflags() { return pImpl->tsflags; }
OptionBool & ConfigMain::assumeyes() { return pImpl->assumeyes; }
OptionBool & ConfigMain::assumeno() { return pImpl->assumeno; }
OptionBool & ConfigMain::check_config_file_age() { return pImpl->check_config_file_age; }
OptionBool & ConfigMain::defaultyes() { return pImpl->defaultyes; }
OptionBool & ConfigMain::diskspacecheck() { return pImpl->diskspacecheck; }
OptionBool & ConfigMain::localpkg_gpgcheck() { return pImpl->localpkg_gpgcheck; }
OptionBool & ConfigMain::gpgkey_dns_verification() { return pImpl->gpgkey_dns_verification; }
OptionBool & ConfigMain::obsoletes() { return pImpl->obsoletes; }
OptionBool & ConfigMain::showdupesfromrepos() { return pImpl->showdupesfromrepos; }
OptionBool & ConfigMain::exit_on_lock() { return pImpl->exit_on_lock; }
OptionSeconds & ConfigMain::metadata_timer_sync() { return pImpl->metadata_timer_sync; }
OptionStringList & ConfigMain::disable_excludes() { return pImpl->disable_excludes; }
OptionEnum<std::string> & ConfigMain::multilib_policy() { return pImpl->multilib_policy; }
OptionBool & ConfigMain::best() { return pImpl->best; }
OptionBool & ConfigMain::install_weak_deps() { return pImpl->install_weak_deps; }
OptionString & ConfigMain::bugtracker_url() { return pImpl->bugtracker_url; }
OptionBool & ConfigMain::zchunk() { return pImpl->zchunk; }
OptionEnum<std::string> & ConfigMain::color() { return pImpl->color; }
OptionString & ConfigMain::color_list_installed_older() { return pImpl->color_list_installed_older; }
OptionString & ConfigMain::color_list_installed_newer() { return pImpl->color_list_installed_newer; }
OptionString & ConfigMain::color_list_installed_reinstall() { return pImpl->color_list_installed_reinstall; }
OptionString & ConfigMain::color_list_installed_extra() { return pImpl->color_list_installed_extra; }
OptionString & ConfigMain::color_list_available_upgrade() { return pImpl->color_list_available_upgrade; }
OptionString & ConfigMain::color_list_available_downgrade() { return pImpl->color_list_available_downgrade; }
OptionString & ConfigMain::color_list_available_reinstall() { return pImpl->color_list_available_reinstall; }
OptionString & ConfigMain::color_list_available_install() { return pImpl->color_list_available_install; }
OptionString & ConfigMain::color_update_installed() { return pImpl->color_update_installed; }
OptionString & ConfigMain::color_update_local() { return pImpl->color_update_local; }
OptionString & ConfigMain::color_update_remote() { return pImpl->color_update_remote; }
OptionString & ConfigMain::color_search_match() { return pImpl->color_search_match; }
OptionBool & ConfigMain::history_record() { return pImpl->history_record; }
OptionStringList & ConfigMain::history_record_packages() { return pImpl->history_record_packages; }
OptionString & ConfigMain::rpmverbosity() { return pImpl->rpmverbosity; }
OptionBool & ConfigMain::strict() { return pImpl->strict; }
OptionBool & ConfigMain::skip_broken() { return pImpl->skip_broken; }
OptionBool & ConfigMain::autocheck_running_kernel() { return pImpl->autocheck_running_kernel; }
OptionBool & ConfigMain::clean_requirements_on_remove() { return pImpl->clean_requirements_on_remove; }
OptionEnum<std::string> & ConfigMain::history_list_view() { return pImpl->history_list_view; }
OptionBool & ConfigMain::upgrade_group_objects_upgrade() { return pImpl->upgrade_group_objects_upgrade; }
OptionPath & ConfigMain::destdir() { return pImpl->destdir; }
OptionString & ConfigMain::comment() { return pImpl->comment; }
OptionBool & ConfigMain::downloadonly() { return pImpl->downloadonly; }
OptionBool & ConfigMain::ignorearch() { return pImpl->ignorearch; }

OptionString & ConfigMain::module_platform_id() { return pImpl->module_platform_id; }
OptionString & ConfigMain::user_agent() { return pImpl->user_agent; }
OptionBool & ConfigMain::countme() { return pImpl->countme; }

// Repo main config
OptionNumber<std::uint32_t> & ConfigMain::retries() { return pImpl->retries; }
OptionString & ConfigMain::cachedir() { return pImpl->cachedir; }
OptionBool & ConfigMain::fastestmirror() { return pImpl->fastestmirror; }
OptionStringList & ConfigMain::excludepkgs() { return pImpl->excludepkgs; }
OptionStringList & ConfigMain::includepkgs() { return pImpl->includepkgs; }
OptionString & ConfigMain::proxy() { return pImpl->proxy; }
OptionString & ConfigMain::proxy_username() { return pImpl->proxy_username; }
OptionString & ConfigMain::proxy_password() { return pImpl->proxy_password; }
OptionEnum<std::string> & ConfigMain::proxy_auth_method() { return pImpl->proxy_auth_method; }
OptionStringList & ConfigMain::protected_packages() { return pImpl->protected_packages; }
OptionString & ConfigMain::username() { return pImpl->username; }
OptionString & ConfigMain::password() { return pImpl->password; }
OptionBool & ConfigMain::gpgcheck() { return pImpl->gpgcheck; }
OptionBool & ConfigMain::repo_gpgcheck() { return pImpl->repo_gpgcheck; }
OptionBool & ConfigMain::enabled() { return pImpl->enabled; }
OptionBool & ConfigMain::enablegroups() { return pImpl->enablegroups; }
OptionNumber<std::uint32_t> & ConfigMain::bandwidth() { return pImpl->bandwidth; }
OptionNumber<std::uint32_t> & ConfigMain::minrate() { return pImpl->minrate; }
OptionEnum<std::string> & ConfigMain::ip_resolve() { return pImpl->ip_resolve; }
OptionNumber<float> & ConfigMain::throttle() { return pImpl->throttle; }
OptionSeconds & ConfigMain::timeout() { return pImpl->timeout; }
OptionNumber<std::uint32_t> & ConfigMain::max_parallel_downloads() { return pImpl->max_parallel_downloads; }
OptionSeconds & ConfigMain::metadata_expire() { return pImpl->metadata_expire; }
OptionString & ConfigMain::sslcacert() { return pImpl->sslcacert; }
OptionBool & ConfigMain::sslverify() { return pImpl->sslverify; }
OptionString & ConfigMain::sslclientcert() { return pImpl->sslclientcert; }
OptionString & ConfigMain::sslclientkey() { return pImpl->sslclientkey; }
OptionBool & ConfigMain::deltarpm() { return pImpl->deltarpm; }
OptionNumber<std::uint32_t> & ConfigMain::deltarpm_percentage() { return pImpl->deltarpm_percentage; }
OptionBool & ConfigMain::skip_if_unavailable() { return pImpl->skip_if_unavailable; }

static void DIRClose(DIR *d) { closedir(d); }

void ConfigMain::addVarsFromDir(std::map<std::string, std::string> & varsMap, const std::string & dirPath)
{
    if (DIR * dir = opendir(dirPath.c_str())) {
        std::unique_ptr<DIR, decltype(&DIRClose)> dirGuard(dir, &DIRClose);
        while (auto ent = readdir(dir)) {
            auto dname = ent->d_name;
            if (dname[0] == '.' && (dname[1] == '\0' || (dname[1] == '.' && dname[2] == '\0')))
                continue;

            auto fullPath = dirPath;
            if (fullPath.back() != '/')
                fullPath += "/";
            fullPath += dname;
            std::ifstream inStream(fullPath);
            if (inStream.fail()) {
                // log.warning()
                continue;
            }
            std::string line;
            std::getline(inStream, line);
            if (inStream.fail()) {
                // log.warning()
                continue;
            }
            varsMap[dname] = std::move(line);
        }
    }
}

void ConfigMain::addVarsFromEnv(std::map<std::string, std::string> & varsMap)
{
    if (!environ)
        return;

    for (const char * const * varPtr = environ; *varPtr; ++varPtr) {
        auto var = *varPtr;
        if (auto eqlPtr = strchr(var, '=')) {
            auto eqlIdx = eqlPtr - var;
            // DNF[0-9]
            if (eqlIdx == 4 && strncmp("DNF", var, 3) == 0 && isdigit(var[3]))
                varsMap[std::string(var, eqlIdx)] = eqlPtr + 1;
            // DNF_VAR_[A-Za-z0-9_]+ , DNF_VAR_ prefix is cut off
            else if (eqlIdx > 8 && strncmp("DNF_VAR_", var, 8) == 0 &&
                     static_cast<int>(strspn(var + 8, ASCII_LETTERS DIGITS "_")) == eqlIdx - 8)
                varsMap[std::string(var + 8, eqlIdx - 8)] = eqlPtr + 1;
        }
    }
}

}
