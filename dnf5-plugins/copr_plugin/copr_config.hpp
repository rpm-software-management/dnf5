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

#ifndef DNF5_COMMANDS_COPR_COPR_CONFIG_HPP
#define DNF5_COMMANDS_COPR_COPR_CONFIG_HPP

#include <libdnf5/base/base.hpp>
#include <libdnf5/conf/config_parser.hpp>

namespace dnf5 {

class CoprConfig : public libdnf5::ConfigParser {
private:
    libdnf5::Base & base;

    void load_copr_config_file(const std::string & filename);
    void load_all_configuration();

public:
    explicit CoprConfig(libdnf5::Base & base);
    std::string get_hub_hostname(const std::string & hubspec);
    std::string get_hub_url(const std::string & hubspec);
    std::string get_repo_url(
        const std::string & hubspec,
        const std::string & ownername,
        const std::string & dirname,
        const std::string & name_version);
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_COPR_COPR_CONFIG_HPP
