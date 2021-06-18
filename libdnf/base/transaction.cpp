/*
Copyright (C) 2021 Red Hat, Inc.

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


#include "transaction_impl.hpp"

#include "libdnf/base/base.hpp"


namespace libdnf::base {

Transaction::Transaction(const BaseWeakPtr & base) : p_impl(new Impl(base)) {}
Transaction::Transaction(const Transaction & transaction) : p_impl(new Impl(*transaction.p_impl)) {}
Transaction::~Transaction() = default;

Transaction::Impl::~Impl() {
    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
    }
}

Transaction::Impl & Transaction::Impl::operator=(const Impl & other) {
    base = other.base;
    libsolv_transaction = other.libsolv_transaction ? transaction_create_clone(other.libsolv_transaction) : nullptr;
    packages = other.packages;
    return *this;
}

const std::vector<TransactionPackageItem> & Transaction::get_packages() const noexcept {
    return p_impl->packages;
}

GoalProblem Transaction::get_problems() {
    return p_impl->problems;
}

std::vector<rpm::Package> Transaction::Impl::list_results(Id type_filter1, Id type_filter2) {
    /* no transaction */
    if (!libsolv_transaction) {
        throw std::runtime_error("no solution possible");
    }

    auto sack = base->get_rpm_package_sack();

    std::vector<rpm::Package> result;
    const int common_mode = SOLVER_TRANSACTION_SHOW_OBSOLETES | SOLVER_TRANSACTION_CHANGE_IS_REINSTALL;

    for (int i = 0; i < libsolv_transaction->steps.count; ++i) {
        Id p = libsolv_transaction->steps.elements[i];
        Id type;

        switch (type_filter1) {
            case SOLVER_TRANSACTION_OBSOLETED:
                type = transaction_type(libsolv_transaction, p, common_mode);
                break;
            default:
                type = transaction_type(
                    libsolv_transaction, p, common_mode | SOLVER_TRANSACTION_SHOW_ACTIVE | SOLVER_TRANSACTION_SHOW_ALL);
                break;
        }

        if (type == type_filter1 || (type_filter2 && type == type_filter2)) {
            result.emplace_back(rpm::Package(sack, rpm::PackageId(p)));
        }
    }
    return result;
}

std::vector<rpm::Package> Transaction::list_rpm_installs() {
    return p_impl->list_results(SOLVER_TRANSACTION_INSTALL, SOLVER_TRANSACTION_OBSOLETES);
}

std::vector<rpm::Package> Transaction::list_rpm_reinstalls() {
    return p_impl->list_results(SOLVER_TRANSACTION_REINSTALL, 0);
}

std::vector<rpm::Package> Transaction::list_rpm_upgrades() {
    return p_impl->list_results(SOLVER_TRANSACTION_UPGRADE, 0);
}

std::vector<rpm::Package> Transaction::list_rpm_downgrades() {
    return p_impl->list_results(SOLVER_TRANSACTION_DOWNGRADE, 0);
}

std::vector<rpm::Package> Transaction::list_rpm_removes() {
    return p_impl->list_results(SOLVER_TRANSACTION_ERASE, 0);
}

std::vector<rpm::Package> Transaction::list_rpm_obsoleted() {
    return p_impl->list_results(SOLVER_TRANSACTION_OBSOLETED, 0);
}

void Transaction::Impl::set_transaction(rpm::solv::GoalPrivate & goal) {
    auto transaction = goal.get_transaction();
    libsolv_transaction = transaction ? transaction_create_clone(transaction) : nullptr;
    if (!libsolv_transaction) {
        return;
    }
    auto sack = base->get_rpm_package_sack();

    // std::map<obsoleted, obsoleted_by>
    std::map<Id, std::vector<Id>> obsoletes;

    auto list_downgrades = goal.list_downgrades();
    for (auto index = 0; index < list_downgrades.size(); ++index) {
        Id id = list_downgrades[index];
        auto obs = goal.list_obsoleted_by_package(id);
        if (obs.empty()) {
            throw RuntimeError("No obsoletes for downgrade");
        }
        rpm::Package new_package(sack, rpm::PackageId(id));
        auto reason = new_package.get_reason();
        TransactionPackageItem item(new_package, TransactionPackageItem::Action::DOWNGRADE, reason);
        item.replace.emplace(rpm::Package(sack, rpm::PackageId(obs[0])));
        for (int i = 1; i < obs.size(); ++i) {
            item.obsoletes.emplace_back(rpm::Package(sack, rpm::PackageId(obs[i])));
            obsoletes[obs[i]].push_back(id);
        }
        packages.emplace_back(std::move(item));

        TransactionPackageItem old_item(
            rpm::Package(sack, rpm::PackageId(obs[0])), TransactionPackageItem::Action::DOWNGRADED, reason);
        packages.emplace_back(std::move(old_item));
    }
    auto list_reinstalls = goal.list_reinstalls();
    for (auto index = 0; index < list_reinstalls.size(); ++index) {
        Id id = list_reinstalls[index];
        auto obs = goal.list_obsoleted_by_package(id);
        if (obs.empty()) {
            throw RuntimeError("No obsoletes for reinstall");
        }
        rpm::Package new_package(sack, rpm::PackageId(id));
        auto reason = new_package.get_reason();
        TransactionPackageItem item(new_package, TransactionPackageItem::Action::REINSTALL, reason);
        item.replace.emplace(rpm::Package(sack, rpm::PackageId(obs[0])));
        for (int i = 1; i < obs.size(); ++i) {
            item.obsoletes.emplace_back(rpm::Package(sack, rpm::PackageId(obs[i])));
            obsoletes[obs[i]].push_back(id);
        }
        packages.emplace_back(std::move(item));

        TransactionPackageItem old_item(
            rpm::Package(sack, rpm::PackageId(obs[0])), TransactionPackageItem::Action::REINSTALLED, reason);
        packages.emplace_back(std::move(old_item));
    }
    auto list_installs = goal.list_installs();
    for (auto index = 0; index < list_installs.size(); ++index) {
        Id id = list_installs[index];
        auto obs = goal.list_obsoleted_by_package(id);
        auto reason = goal.get_reason(id);
        TransactionPackageItem item(
            rpm::Package(sack, rpm::PackageId(id)), TransactionPackageItem::Action::REINSTALL, reason);
        for (int i = 0; i < obs.size(); ++i) {
            item.obsoletes.emplace_back(rpm::Package(sack, rpm::PackageId(obs[i])));
            obsoletes[obs[i]].push_back(id);
        }
        // TODO(jmracek) Missing a lot of conditions
        packages.emplace_back(std::move(item));
    }
    auto list_upgrades = goal.list_upgrades();
    for (auto index = 0; index < list_upgrades.size(); ++index) {
        Id id = list_upgrades[index];
        auto obs = goal.list_obsoleted_by_package(id);
        if (obs.empty()) {
            throw RuntimeError("No obsoletes for reinstall");
        }
        rpm::Package new_package(sack, rpm::PackageId(id));
        auto reason = new_package.get_reason();
        TransactionPackageItem item(new_package, TransactionPackageItem::Action::UPGRADE, reason);
        item.replace.emplace(rpm::Package(sack, rpm::PackageId(obs[0])));
        for (int i = 1; i < obs.size(); ++i) {
            item.obsoletes.emplace_back(rpm::Package(sack, rpm::PackageId(obs[i])));
            obsoletes[obs[i]].push_back(id);
        }
        packages.emplace_back(std::move(item));

        TransactionPackageItem old_item(
            rpm::Package(sack, rpm::PackageId(obs[0])), TransactionPackageItem::Action::UPGRADED, reason);
        packages.emplace_back(std::move(old_item));
    }
    auto list_removes = goal.list_removes();
    for (auto index = 0; index < list_removes.size(); ++index) {
        Id id = list_removes[index];
        rpm::Package new_package(sack, rpm::PackageId(id));
        auto reason = goal.get_reason(id);
        TransactionPackageItem item(
            rpm::Package(sack, rpm::PackageId(id)), TransactionPackageItem::Action::REMOVE, reason);
        packages.emplace_back(std::move(item));
        // TODO(jmracek) Missing a lot of conditions
    }

    // Add obsoleted packages
    for (const auto & [obsoleted_id, obsoleted_by_ids] : obsoletes) {
        rpm::Package obsoleted(sack, rpm::PackageId(obsoleted_id));
        auto reason = goal.get_reason(obsoleted_id);
        TransactionPackageItem tsi(obsoleted, TransactionPackageItem::Action::OBSOLETED, reason);
        for (auto id : obsoleted_by_ids) {
            tsi.obsoletes.emplace_back(rpm::Package(sack, rpm::PackageId(id)));
        }
    }
}


//     def _goal2transaction(self, goal):
//         ts = self.history.rpm
//         all_obsoleted = set(goal.list_obsoleted())
//         installonly_query = self._get_installonly_query()
//         installonly_query.apply()
//         installonly_query_installed = installonly_query.installed().apply()
//
//         for pkg in goal.list_downgrades():
//             obs = goal.obsoleted_by_package(pkg)
//             downgraded = obs[0]
//             self._ds_callback.pkg_added(downgraded, 'dd')
//             self._ds_callback.pkg_added(pkg, 'd')
//             ts.add_downgrade(pkg, downgraded, obs[1:])
//         for pkg in goal.list_reinstalls():
//             self._ds_callback.pkg_added(pkg, 'r')
//             obs = goal.obsoleted_by_package(pkg)
//             nevra_pkg = str(pkg)
//             # reinstall could obsolete multiple packages with the same NEVRA or different NEVRA
//             # Set the package with the same NEVRA as reinstalled
//             obsoletes = []
//             for obs_pkg in obs:
//                 if str(obs_pkg) == nevra_pkg:
//                     obsoletes.insert(0, obs_pkg)
//                 else:
//                     obsoletes.append(obs_pkg)
//             reinstalled = obsoletes[0]
//             ts.add_reinstall(pkg, reinstalled, obsoletes[1:])
//         for pkg in goal.list_installs():
//             self._ds_callback.pkg_added(pkg, 'i')
//             obs = goal.obsoleted_by_package(pkg)
//             # Skip obsoleted packages that are not part of all_obsoleted,
//             # they are handled as upgrades/downgrades.
//             # Also keep RPMs with the same name - they're not always in all_obsoleted.
//             obs = [i for i in obs if i in all_obsoleted or i.name == pkg.name]
//
//             reason = goal.get_reason(pkg)
//
//             #  Inherit reason if package is installonly an package with same name is installed
//             #  Use the same logic like upgrade
//             #  Upgrade of installonly packages result in install or install and remove step
//             if pkg in installonly_query and installonly_query_installed.filter(name=pkg.name):
//                 reason = ts.get_reason(pkg)
//
//             # inherit the best reason from obsoleted packages
//             for obsolete in obs:
//                 reason_obsolete = ts.get_reason(obsolete)
//                 if libdnf.transaction.TransactionItemReasonCompare(reason, reason_obsolete) == -1:
//                     reason = reason_obsolete
//
//             ts.add_install(pkg, obs, reason)
//             cb = lambda pkg: self._ds_callback.pkg_added(pkg, 'od')
//             dnf.util.mapall(cb, obs)
//         for pkg in goal.list_upgrades():
//             obs = goal.obsoleted_by_package(pkg)
//             upgraded = None
//             for i in obs:
//                 # try to find a package with matching name as the upgrade
//                 if i.name == pkg.name:
//                     upgraded = i
//                     break
//             if upgraded is None:
//                 # no matching name -> pick the first one
//                 upgraded = obs.pop(0)
//             else:
//                 obs.remove(upgraded)
//             # Skip obsoleted packages that are not part of all_obsoleted,
//             # they are handled as upgrades/downgrades.
//             # Also keep RPMs with the same name - they're not always in all_obsoleted.
//             obs = [i for i in obs if i in all_obsoleted or i.name == pkg.name]
//
//             cb = lambda pkg: self._ds_callback.pkg_added(pkg, 'od')
//             dnf.util.mapall(cb, obs)
//             if pkg in installonly_query:
//                 ts.add_install(pkg, obs)
//             else:
//                 ts.add_upgrade(pkg, upgraded, obs)
//                 self._ds_callback.pkg_added(upgraded, 'ud')
//             self._ds_callback.pkg_added(pkg, 'u')
//         erasures = goal.list_erasures()
//         if erasures:
//             remaining_installed_query = self.sack.query(flags=hawkey.IGNORE_EXCLUDES).installed()
//             remaining_installed_query.filterm(pkg__neq=erasures)
//             for pkg in erasures:
//                 if remaining_installed_query.filter(name=pkg.name):
//                     remaining = remaining_installed_query[0]
//                     ts.get_reason(remaining)
//                     self.history.set_reason(remaining, ts.get_reason(remaining))
//                 self._ds_callback.pkg_added(pkg, 'e')
//                 reason = goal.get_reason(pkg)
//                 ts.add_erase(pkg, reason)
//         return ts


}  // namespace libdnf::base
