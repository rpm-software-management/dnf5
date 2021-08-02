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

#ifndef MICRODNF_COMMANDS_REPOQUERY_REPOQUERY_HPP
#define MICRODNF_COMMANDS_REPOQUERY_REPOQUERY_HPP

#include "../command.hpp"

#include <libdnf/conf/option_bool.hpp>

#include <memory>
#include <vector>

namespace microdnf {

class CmdRepoquery : public Command {
public:
    void set_argument_parser(Context & ctx) override;
    void run(Context & ctx) override;

private:
    libdnf::OptionBool * available_option{nullptr};
    libdnf::OptionBool * installed_option{nullptr};
    libdnf::OptionBool * info_option{nullptr};
    libdnf::OptionBool * nevra_option{nullptr};
    std::vector<std::unique_ptr<libdnf::Option>> * patterns_to_show_options{nullptr};
};

}  // namespace microdnf

#endif
