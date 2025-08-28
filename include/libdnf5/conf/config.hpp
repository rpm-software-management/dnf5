// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CONF_CONFIG_HPP
#define LIBDNF5_CONF_CONFIG_HPP

#include "config_parser.hpp"
#include "option.hpp"
#include "option_binds.hpp"
#include "vars.hpp"

#include "libdnf5/defs.h"
#include "libdnf5/logger/logger.hpp"


namespace libdnf5 {

/// Base class for configurations objects
class LIBDNF_API Config {
public:
    OptionBinds & opt_binds() noexcept;

    Config();
    virtual ~Config();

    virtual void load_from_parser(
        const ConfigParser & parser,
        const std::string & section,
        const Vars & vars,
        Logger & logger,
        Option::Priority priority);

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


}  // namespace libdnf5

#endif
