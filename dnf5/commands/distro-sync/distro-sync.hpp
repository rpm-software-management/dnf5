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


#ifndef DNF5_COMMANDS_DISTRO_SYNC_DISTRO_SYNC_HPP
#define DNF5_COMMANDS_DISTRO_SYNC_DISTRO_SYNC_HPP

#include <dnf5/context.hpp>
#include <libdnf/conf/option_bool.hpp>

#include <memory>
#include <vector>


namespace dnf5 {


class DistroSyncCommand : public Command {
public:
    explicit DistroSyncCommand(Command & parent) : Command(parent, "distro-sync") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

    std::vector<std::unique_ptr<libdnf::Option>> * patterns_to_distro_sync_options{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_DISTRO_SYNC_DISTRO_SYNC_HPP
