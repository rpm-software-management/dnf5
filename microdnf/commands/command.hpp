/*
Copyright (C) 2019-2020 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MICRODNF_COMMANDS_COMMAND_HPP
#define MICRODNF_COMMANDS_COMMAND_HPP

namespace microdnf {

class Context;

class Command {
public:
    virtual void set_argument_parser(Context &) {}
    virtual void pre_configure(Context &) {}
    virtual void configure(Context &) {}
    virtual void run(Context &) {}
    virtual ~Command() = default;
};

}  // namespace microdnf

#endif
