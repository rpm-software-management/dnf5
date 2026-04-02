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

#ifndef DNF5_COMMANDS_OBS_OBS_CONFIG_HPP
#define DNF5_COMMANDS_OBS_OBS_CONFIG_HPP

#include <libdnf5/base/base.hpp>
#include <libdnf5/conf/config_parser.hpp>

namespace dnf5 {

class ObsConfig : public libdnf5::ConfigParser {
private:
    libdnf5::Base & base;

    void load_builtin_config();
    void load_obs_config_file(const std::string & filename);
    void load_all_configuration();

    std::string get_option_value(
        const std::string & section,
        const std::string & option,
        const std::string & default_value = "",
        bool use_default_section = true);

    std::string get_url(const std::string & hostname, const std::string & url_type);

public:
    explicit ObsConfig(libdnf5::Base & base);
    std::string get_hub_hostname(const std::string & hubspec);
    std::string get_html_repository_state_url(
        const std::string & hostname,
        const std::string & project,
        const std::string & reponame);
    std::string get_download_url(
        const std::string & hostname,
        const std::string & project,
        const std::string & reponame);
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_OBS_OBS_CONFIG_HPP
