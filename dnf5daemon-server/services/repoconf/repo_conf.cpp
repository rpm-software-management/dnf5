/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "repo_conf.hpp"

#include "configuration.hpp"
#include "dbus.hpp"
#include "utils.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <string>

void RepoConf::dbus_register() {
    auto dbus_object = session.get_dbus_object();
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_REPOCONF, "list", "a{sv}", "aa{sv}", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &RepoConf::list, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_REPOCONF, "get", "s", "a{sv}", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &RepoConf::get, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_REPOCONF, "enable", "as", "as", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &RepoConf::enable, call, session.session_locale);
        });
    dbus_object->registerMethod(
        dnfdaemon::INTERFACE_REPOCONF, "disable", "as", "as", [this](sdbus::MethodCall call) -> void {
            session.get_threads_manager().handle_method(*this, &RepoConf::disable, call, session.session_locale);
        });
}

dnfdaemon::KeyValueMapList RepoConf::repo_list(const std::vector<std::string> & ids) {
    Configuration cfg(session);
    cfg.read_configuration();

    bool empty_ids = ids.empty();
    dnfdaemon::KeyValueMapList out;
    for (auto & repo : cfg.get_repos()) {
        if (empty_ids || std::find(ids.begin(), ids.end(), repo.first) != ids.end()) {
            auto parser = cfg.find_parser(repo.second->file_path);
            if (parser) {
                dnfdaemon::KeyValueMap dbus_repo;
                dbus_repo.emplace(std::make_pair("repoid", repo.first));
                for (const auto & section : parser->get_data()) {
                    if (section.first == repo.first) {
                        for (const auto & line : section.second) {
                            if (line.first[0] != '#') {
                                dbus_repo.emplace(std::make_pair(line.first, line.second));
                            }
                        }
                    }
                }
                out.push_back(dbus_repo);
            }
        }
    }
    return out;
}

sdbus::MethodReply RepoConf::list(sdbus::MethodCall & call) {
    dnfdaemon::KeyValueMap options;
    std::vector<std::string> default_ids{};
    call >> options;
    std::vector<std::string> ids = key_value_map_get<std::vector<std::string>>(options, "ids", default_ids);
    auto out = repo_list(std::move(ids));
    auto reply = call.createReply();
    reply << out;

    return reply;
}

sdbus::MethodReply RepoConf::get(sdbus::MethodCall & call) {
    std::string id;
    call >> id;
    auto ids = std::vector<std::string>{std::move(id)};
    auto lst = repo_list(ids);
    if (lst.empty()) {
        throw sdbus::Error(dnfdaemon::ERROR_REPOCONF, "Repository not found");
    } else if (lst.size() > 1) {
        throw sdbus::Error(dnfdaemon::ERROR_REPOCONF, "Multiple repositories found");
    } else {
        auto reply = call.createReply();
        reply << lst[0];
        return reply;
    }
}

std::vector<std::string> RepoConf::enable_disable_repos(const std::vector<std::string> & ids, const bool enable) {
    Configuration cfg(session);
    cfg.read_configuration();

    std::vector<std::string> out;
    std::vector<std::string> changed_config_files;
    for (auto & repoid : ids) {
        auto repoinfo = cfg.find_repo(repoid);
        if (repoinfo && repoinfo->repoconfig->get_enabled_option().get_value() != enable) {
            auto parser = cfg.find_parser(repoinfo->file_path);
            if (parser) {
                parser->set_value(repoid, "enabled", enable ? "1" : "0");
                changed_config_files.push_back(repoinfo->file_path);
                out.push_back(repoid);
            }
        }
    }
    for (auto & config_file : changed_config_files) {
        try {
            cfg.find_parser(config_file)->write(config_file, false);
        } catch (std::exception & e) {
            throw sdbus::Error(
                dnfdaemon::ERROR_REPOCONF, std::string("Unable to write configuration file: ") + e.what());
        }
    }

    return out;
}

sdbus::MethodReply RepoConf::enable_disable(sdbus::MethodCall && call, const bool & enable) {
    auto sender = call.getSender();
    std::vector<std::string> ids;
    call >> ids;
    auto is_authorized = session.check_authorization(dnfdaemon::POLKIT_REPOCONF_WRITE, sender);
    if (!is_authorized) {
        throw sdbus::Error(dnfdaemon::ERROR_REPOCONF, "Not authorized.");
    }
    auto reply = call.createReply();
    reply << enable_disable_repos(ids, enable);
    return reply;
}
