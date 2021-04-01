/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNFDAEMON_CLIENT_COMMANDS_REPOQUERY_REPOQUERY_HPP
#define DNFDAEMON_CLIENT_COMMANDS_REPOQUERY_REPOQUERY_HPP

#include "../command.hpp"

#include <libdnf/conf/option_bool.hpp>

#include <memory>
#include <vector>

namespace dnfdaemon::client {

class CmdRepoquery : public Command {
public:
    void set_argument_parser(Context & ctx) override;
    void run(Context & ctx) override;
    dnfdaemon::KeyValueMap session_config(Context &) override;

private:
    libdnf::OptionBool * available_option{nullptr};
    libdnf::OptionBool * installed_option{nullptr};
    libdnf::OptionBool * info_option{nullptr};
    std::vector<std::unique_ptr<libdnf::Option>> * patterns_options{nullptr};
};

}  // namespace dnfdaemon::client

#endif
