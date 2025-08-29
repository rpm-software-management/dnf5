// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
