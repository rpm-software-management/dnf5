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

#include "libdnf5/conf/config.hpp"


namespace libdnf5 {

class Config::Impl {
public:
    Impl() = default;
    Impl(const Impl & other) : binds(other.binds) {}

    Impl & operator=(const Impl & other) {
        binds = other.binds;
        return *this;
    }

private:
    friend Config;

    OptionBinds binds;
};

OptionBinds & Config::opt_binds() noexcept {
    return p_impl->binds;
}

Config::Config() : p_impl(new Impl()) {}

Config::Config(const Config & other) : p_impl(other.p_impl) {}

Config::~Config() = default;

Config & Config::operator=(const Config & other) {
    if (this != &other) {
        p_impl = other.p_impl;
    }
    return *this;
}

void Config::load_from_parser(
    const ConfigParser & parser,
    const std::string & section,
    const Vars & vars,
    Logger & logger,
    Option::Priority priority) {
    auto cfg_parser_data_iter = parser.get_data().find(section);
    if (cfg_parser_data_iter != parser.get_data().end()) {
        for (const auto & opt : cfg_parser_data_iter->second) {
            auto opt_binds_iter = p_impl->binds.find(opt.first);
            if (opt_binds_iter != p_impl->binds.end()) {
                try {
                    opt_binds_iter->second.new_string(priority, vars.substitute(opt.second));
                } catch (const OptionError & ex) {
                    logger.warning("Config error in section \"{}\" key \"{}\": {}", section, opt.first, ex.what());
                }
            }
        }
    }
}

}  // namespace libdnf5
