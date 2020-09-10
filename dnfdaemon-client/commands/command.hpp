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

#ifndef DNFDAEMON_CLIENT_COMMANDS_COMMAND_HPP
#define DNFDAEMON_CLIENT_COMMANDS_COMMAND_HPP

namespace dnfdaemon::client {

class Context;

class Command {
public:
    virtual void set_argument_parser(Context &) {}
    virtual void pre_configure(Context &) {}
    virtual void configure(Context &) {}
    virtual void run(Context &) {}
    virtual ~Command() = default;
};

}  // namespace dnfdaemon::client

#endif
