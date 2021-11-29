/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_CONF_CONFIG_HPP
#define LIBDNF_CONF_CONFIG_HPP

#include "config_parser.hpp"
#include "option_binds.hpp"
#include "option.hpp"
#include "vars.hpp"
#include "../logger/logger.hpp"


namespace libdnf {

/// Base class for configurations objects
template <Option::Priority default_priority>
class Config {
public:
    OptionBinds & opt_binds() noexcept { return binds; }

    virtual ~Config() = default;

    virtual void load_from_parser(
        const ConfigParser & parser,
        const std::string & section,
        const Vars & vars,
        Logger & logger
    );

private:
    OptionBinds binds;
};

extern template class Config<Option::Priority::MAINCONFIG>;
extern template class Config<Option::Priority::REPOCONFIG>;

}  // namespace libdnf

#endif
