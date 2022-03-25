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


#ifndef MICRODNF_COMMANDS_SWAP_SWAP_HPP
#define MICRODNF_COMMANDS_SWAP_SWAP_HPP


#include <libdnf-cli/session.hpp>

#include <vector>


namespace microdnf {


// TODO(jrohel): The "swap" command may be removed in the future in favor of a more powerful command (eg "do"),
//               which will allow multiple actions to be combined in one transaction.
class SwapCommand : public libdnf::cli::session::Command {
public:
    explicit SwapCommand(Command & parent);
    void run() override;

private:
    std::string remove_pkg_spec;
    std::vector<std::string> install_pkg_specs;
    std::vector<std::string> install_pkg_file_paths;
};


}  // namespace microdnf


#endif  // MICRODNF_COMMANDS_SWAP_SWAP_HPP
