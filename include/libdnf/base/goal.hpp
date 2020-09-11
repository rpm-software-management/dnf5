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

#ifndef LIBDNF_BASE_GOAL_HPP
#define LIBDNF_BASE_GOAL_HPP

#include "base.hpp"

#include "libdnf/rpm/nevra.hpp"
#include "libdnf/rpm/package.hpp"

#include <memory>
#include <tuple>
#include <vector>

namespace libdnf {


class Goal {
public:
    /// NOT_FOUND - _('No match for argument: %s')
    enum class Problem { NOT_FOUND, EXCLUDED, ONLY_SRC, NOT_FOUND_IN_REPOSITORIES };
    enum class Action { INSTALL, UPGRADE, REMOVE };

    explicit Goal(Base * base);
    ~Goal();
    void add_module_enable(const std::string & spec);
    void add_rpm_install(
        const std::string & spec,
        const std::vector<std::string> & repo_ids,
        bool strict,
        const std::vector<libdnf::rpm::Nevra::Form> & forms);
    void add_rpm_install(const libdnf::rpm::Package & rpm_package, bool strict);
    void add_rpm_install(const libdnf::rpm::PackageSet & package_set, bool strict);
    void add_rpm_install(const libdnf::rpm::SolvQuery & query, bool strict);
    void add_rpm_remove(
        const std::string & spec, const std::string & repo_id, const std::vector<libdnf::rpm::Nevra::Form> & forms);
    void add_rpm_remove(const libdnf::rpm::Package & rpm_package);
    void add_rpm_remove(const libdnf::rpm::PackageSet & package_set);
    void add_rpm_remove(const libdnf::rpm::SolvQuery & query);
    void add_rpm_upgrade(const std::string & spec, const std::vector<std::string> & repo_ids);
    void add_rpm_upgrade(const libdnf::rpm::Package & rpm_package);
    void add_rpm_upgrade(const libdnf::rpm::PackageSet & package_set);
    void add_rpm_upgrade(const libdnf::rpm::SolvQuery & query);

    bool resolve();

    std::vector<libdnf::rpm::Package> list_rpm_installs();
    std::vector<libdnf::rpm::Package> list_rpm_reinstalls();
    std::vector<libdnf::rpm::Package> list_rpm_upgrades();
    std::vector<libdnf::rpm::Package> list_rpm_downgrades();
    std::vector<libdnf::rpm::Package> list_rpm_removes();
    std::vector<libdnf::rpm::Package> list_rpm_obsoleted();

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf

#endif  // LIBDNF_BASE_GOAL_HPP
