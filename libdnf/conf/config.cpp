/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf/conf/config.hpp"

#include <fmt/format.h>

namespace libdnf {

template <Option::Priority default_priority>
void Config<default_priority>::load_from_parser(
    const ConfigParser & parser,
    const std::string & section,
    const Vars & vars,
    Logger & logger
) {
    auto cfg_parser_data_iter = parser.get_data().find(section);
    if (cfg_parser_data_iter != parser.get_data().end()) {
        for (const auto & opt : cfg_parser_data_iter->second) {
            auto opt_binds_iter = binds.find(opt.first);
            if (opt_binds_iter != binds.end()) {
                try {
                    opt_binds_iter->second.new_string(default_priority, vars.substitute(opt.second));
                } catch (const Option::Exception & ex) {
                    logger.warning(fmt::format(
                        R"**(Config error in section "{}" key "{}": {}: {})**",
                        section,
                        opt.first,
                        ex.get_description(),
                        ex.what()
                    ));
                }
            }
        }
    }
}

template class Config<Option::Priority::MAINCONFIG>;
template class Config<Option::Priority::REPOCONFIG>;

}
