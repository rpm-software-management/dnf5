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

#ifndef LIBDNF_BASE_BASE_HPP
#define LIBDNF_BASE_BASE_HPP

#include "libdnf/conf/config_main.hpp"
#include "libdnf/logger/log_router.hpp"
#include "libdnf/rpm/repo_sack.hpp"
#include "libdnf/rpm/solv_sack.hpp"

#include <map>

namespace libdnf {


/// Instances of :class:`libdnf::Base` are the central point of functionality supplied by libdnf.
/// An application will typically create a single instance of this class which it will keep for the run-time needed to accomplish its packaging tasks.
/// :class:`.Base` instances are stateful objects owning various data.
class Base {
public:
    ConfigMain & get_config() { return config; }
    LogRouter & get_logger() { return log_router; }
    rpm::RepoSack & get_rpm_repo_sack() { return rpm_repo_sack; }
    rpm::SolvSack & get_rpm_solv_sack() { return rpm_solv_sack; }

    /// Gets base variables. They can be used in configuration files. Syntax in the config - ${var_name} or $var_name.
    std::map<std::string, std::string> & get_variables() { return variables; }

private:
    ConfigMain config;
    LogRouter log_router;
    rpm::RepoSack rpm_repo_sack{*this};
    rpm::SolvSack rpm_solv_sack{*this};
    std::map<std::string, std::string> variables;
};


}  // namespace libdnf

#endif
