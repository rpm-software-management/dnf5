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

#ifndef DNFDAEMON_CLIENT_COMMANDS_REPOLIST_REPOLIST_HPP
#define DNFDAEMON_CLIENT_COMMANDS_REPOLIST_REPOLIST_HPP

#include "../command.hpp"

#include <libdnf/conf/option_enum.hpp>

#include <memory>
#include <vector>

namespace dnfdaemon::client {

class CmdRepolist : public Command {
public:
    void set_argument_parser(Context & ctx) override;
    void run(Context & ctx) override;

private:
    libdnf::OptionEnum<std::string> * enable_disable_option{nullptr};
    std::vector<std::unique_ptr<libdnf::Option>> * patterns_to_show_options{nullptr};
};

}  // namespace dnfdaemon::client

#endif
