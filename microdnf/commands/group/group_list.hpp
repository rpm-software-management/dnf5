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

#ifndef MICRODNF_COMMANDS_GROUP_GROUP_LIST_HPP
#define MICRODNF_COMMANDS_GROUP_GROUP_LIST_HPP


#include "arguments.hpp"

#include <libdnf-cli/session.hpp>
#include <libdnf/conf/option_bool.hpp>

#include <memory>
#include <vector>


namespace microdnf {


class GroupListCommand : public libdnf::cli::session::Command {
public:
    explicit GroupListCommand(Command & parent);
    void run() override;

    std::unique_ptr<GroupAvailableOption> available{nullptr};
    std::unique_ptr<GroupInstalledOption> installed{nullptr};
    std::unique_ptr<GroupHiddenOption> hidden{nullptr};
    std::unique_ptr<GroupSpecArguments> group_specs{nullptr};

protected:
    // to be used by an alias command only
    explicit GroupListCommand(Command & parent, const std::string & name);
};


}  // namespace microdnf


#endif  // MICRODNF_COMMANDS_GROUP_GROUP_LIST_HPP
