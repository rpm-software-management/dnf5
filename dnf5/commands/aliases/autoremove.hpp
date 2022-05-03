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


#ifndef DNF5_COMMANDS_ALIASES_AUTOREMOVE_HPP
#define DNF5_COMMANDS_ALIASES_AUTOREMOVE_HPP


#include "commands/remove/remove.hpp"


namespace dnf5 {


class AutoremoveAlias : public RemoveCommand {
public:
    explicit AutoremoveAlias(Command & parent) : RemoveCommand(parent, "autoremove") {
        auto & cmd = *get_argument_parser_command();
        cmd.set_short_description("Alias for 'remove --unneeded'");

        // set the default value of the --unneeded option to `true`
        auto unneeded = dynamic_cast<libdnf::OptionBool *>(this->unneeded);
        unneeded->set(libdnf::Option::Priority::DEFAULT, true);
    }
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_ALIASES_AUTOREMOVE_HPP
