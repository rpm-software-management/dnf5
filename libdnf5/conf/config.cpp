// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/conf/config.hpp"


namespace libdnf5 {

class Config::Impl {
private:
    friend Config;

    OptionBinds binds;
};

OptionBinds & Config::opt_binds() noexcept {
    return p_impl->binds;
}

Config::~Config() = default;
Config::Config() : p_impl(new Impl()) {}

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
