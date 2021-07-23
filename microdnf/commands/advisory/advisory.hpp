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

#ifndef MICRODNF_COMMANDS_ADVISORY_ADVISORY_HPP
#define MICRODNF_COMMANDS_ADVISORY_ADVISORY_HPP

#include "../command.hpp"

#include <libdnf/conf/option_bool.hpp>
#include <libdnf/conf/option_enum.hpp>

#include <memory>
#include <vector>

namespace microdnf {

class CmdAdvisory : public Command {
public:
    void set_argument_parser(Context & ctx) override;
    void run(Context & ctx) override;

private:
    libdnf::OptionBool * with_cve_option{nullptr};
    libdnf::OptionBool * with_bz_option{nullptr};
    libdnf::OptionEnum<std::string> * availability_option{nullptr};
    libdnf::OptionEnum<std::string> * output_type_option{nullptr};
    std::vector<std::unique_ptr<libdnf::Option>> * patterns_to_show_options{nullptr};
};

}  // namespace microdnf

#endif
