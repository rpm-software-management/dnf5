// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_MODULE_MODULE_GOAL_PRIVATE_HPP
#define LIBDNF5_MODULE_MODULE_GOAL_PRIVATE_HPP

#include "solv/id_queue.hpp"
#include "solv/solver.hpp"

#include "libdnf5/module/module_sack.hpp"
#include "libdnf5/module/module_sack_weak.hpp"

extern "C" {
#include <solv/solver.h>
}

#include "libdnf5/base/transaction.hpp"

namespace libdnf5::module {


/// Private part of the module goal that works directly with libsolv.
class ModuleGoalPrivate {
public:
    explicit ModuleGoalPrivate(const ModuleSackWeakPtr & module_sack) : module_sack(module_sack) {}
    explicit ModuleGoalPrivate(const ModuleGoalPrivate & src);
    ~ModuleGoalPrivate();
    ModuleGoalPrivate & operator=(const ModuleGoalPrivate & src);

    /// Adds an install operation for given provide.
    ///
    /// @param reldepid Id of the reldep to install.
    /// @param skip_broken Whether solver can skip reldep with unmet dependencies
    /// @param best Whether the latest version is required or not.
    /// @since 5.0
    void add_provide_install(Id reldepid, bool skip_broken, bool best);

    /// Adds an install operation for one of the items.
    ///
    /// @param queue Queue of module solvable ids.
    /// @param skip_broken Whether solver can skip reldep with unmet dependencies
    /// @param best Whether the latest version is required or not.
    /// @since 5.0.14
    void add_install(libdnf5::solv::IdQueue & queue, bool skip_broken, bool best);

    /// Resolve all operations.
    ///
    /// @return libdnf5::GoalProblem to indicate whether there was a solver error.
    /// @since 5.0
    libdnf5::GoalProblem resolve();

    /// @return `std::vector` of problems during resolving. Each problem is a `std::vector` of items:
    ///          ProblemRules, source, dependency, target, solv string.
    ///          The goal must be resolved first.
    /// @since 5.0
    std::vector<std::vector<std::tuple<ProblemRules, Id, Id, Id, std::string>>> get_problems();

    /// @return IdQueue of items to install. The goal must be resolved first.
    /// @since 5.0
    libdnf5::solv::IdQueue list_installs();
    /// @return IdQueue of items that conflict. The goal must be resolved first.
    /// @since 5.0
    libdnf5::solv::IdQueue list_conflicting();

    /// @return Transaction object. The goal must be resolved first.
    /// @since 5.0
    ::Transaction * get_transaction() { return libsolv_transaction; }

    /// Write solver debug data to the given directory.
    /// @param abs_dest_dir Destination directory. Requires a full existing path.
    void write_debugdata(const std::filesystem::path & abs_dest_dir);

private:
    libdnf5::solv::IdQueue list_results(Id type_filter1, Id type_filter2);

    ModuleSackWeakPtr module_sack;

    libdnf5::solv::IdQueue staging;
    libdnf5::solv::Solver libsolv_solver;

    ::Transaction * libsolv_transaction{nullptr};
};


inline ModuleGoalPrivate::ModuleGoalPrivate(const ModuleGoalPrivate & src)
    : module_sack(src.module_sack),
      staging(src.staging) {}


inline ModuleGoalPrivate::~ModuleGoalPrivate() {
    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
    }
}


inline ModuleGoalPrivate & ModuleGoalPrivate::operator=(const ModuleGoalPrivate & src) {
    if (this != &src) {
        module_sack = src.module_sack;
        staging = src.staging;
        if (libsolv_solver.is_initialized()) {
            libsolv_solver.reset();
        }
        if (libsolv_transaction != nullptr) {
            transaction_free(libsolv_transaction);
            libsolv_transaction = nullptr;
        }
    }
    return *this;
}


}  // namespace libdnf5::module


#endif  // LIBDNF5_MODULE_MODULE_GOAL_PRIVATE_HPP
