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
#include "libdnf/conf/vars.hpp"
#include "libdnf/transaction/sack.hpp"

#include <map>

namespace libdnf {


/// Instances of :class:`libdnf::Base` are the central point of functionality supplied by libdnf.
/// An application will typically create a single instance of this class which it will keep for the run-time needed to accomplish its packaging tasks.
/// :class:`.Base` instances are stateful objects owning various data.
class Base {
public:
    /// Sets the pointer to the locked instance "Base" to "this" instance. Blocks if the pointer is already set.
    /// Pointer to a locked "Base" instance can be obtained using "get_locked_base()".
    void lock();

    /// Resets the pointer to a locked "Base" instance to "nullptr".
    /// Throws an exception if another or no instance is locked.
    void unlock();

    /// Returns a pointer to a locked "Base" instance or "nullptr" if no instance is locked.
    static Base * get_locked_base() noexcept;

    /// Loads main configuration from file defined by the current configuration.
    void load_config_from_file();

    ConfigMain & get_config() { return config; }
    LogRouter & get_logger() { return log_router; }
    rpm::RepoSack & get_rpm_repo_sack() { return rpm_repo_sack; }
    rpm::SolvSack & get_rpm_solv_sack() { return rpm_solv_sack; }
    transaction::TransactionSack & get_transaction_sack() { return transaction_sack; }

    /// Gets base variables. They can be used in configuration files. Syntax in the config - ${var_name} or $var_name.
    Vars & get_vars() { return vars; }

private:
    //TODO(jrohel): Make public?
    /// Loads main configuration from file defined by path.
    void load_config_from_file(const std::string & path);

    //TODO(jrohel): Make public? Will we support drop-in configuration directories?
    /// Loads main configuration from files with ".conf" extension from directory defined by dir_path.
    /// The files in the directory are read in alphabetical order.
    void load_config_from_dir(const std::string & dir_path);

    //TODO(jrohel): Make public? Will we support drop-in configuration directories?
    /// Loads main configuration from files with ".conf" extension from directory defined by the current configuration.
    /// The files in the directory are read in alphabetical order.
    void load_config_from_dir();

    ConfigMain config;
    LogRouter log_router;
    rpm::RepoSack rpm_repo_sack{*this};
    rpm::SolvSack rpm_solv_sack{*this};
    transaction::TransactionSack transaction_sack{*this};
    Vars vars;
};


}  // namespace libdnf

#endif
