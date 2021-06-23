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
#include "libdnf/rpm/package_query.hpp"
#include "libdnf/rpm/package_set_impl.hpp"
#include "libdnf/rpm/solv/package_private.hpp"


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

GoalProblem Transaction::get_problems() {
    return p_impl->problems;
}

std::vector<TransactionPackage> Transaction::get_packages() {
    return p_impl->packages;
}

void Transaction::Impl::set_transaction(rpm::solv::GoalPrivate & solved_goal, GoalProblem problems) {
    this->problems = problems;
    auto transaction = solved_goal.get_transaction();
    libsolv_transaction = transaction ? transaction_create_clone(transaction) : nullptr;
    if (!libsolv_transaction) {
        return;
    }
    auto sack = base->get_rpm_package_sack();
    Pool * pool = sack->p_impl->get_pool();

    rpm::PackageQuery installonly_query(base, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
    installonly_query.filter_provides(base->get_config().installonlypkgs().get_value());
    rpm::PackageQuery installed_installonly_query(installonly_query);
    installed_installonly_query.filter_installed();

    // std::map<obsoleted, obsoleted_by>
    std::map<Id, std::vector<Id>> obsoletes;

    auto list_downgrades = solved_goal.list_downgrades();
    for (auto index = 0; index < list_downgrades.size(); ++index) {
        Id id = list_downgrades[index];
        auto obs = solved_goal.list_obsoleted_by_package(id);
        if (obs.empty()) {
            throw RuntimeError("No obsoletes for downgrade");
        }
        rpm::Package new_package(sack, rpm::PackageId(id));
        auto reason = new_package.get_reason();
        TransactionPackage tspkg(new_package, TransactionPackage::Action::DOWNGRADE, reason);
        tspkg.replaces.emplace(rpm::Package(sack, rpm::PackageId(obs[0])));
        for (int i = 1; i < obs.size(); ++i) {
            rpm::Package obsoleted(sack, rpm::PackageId(obs[i]));
            auto obs_reson = obsoleted.get_reason();
            if (obs_reson > reason) {
                reason = obs_reson;
            }
            tspkg.obsoletes.emplace_back(std::move(obsoleted));
            obsoletes[obs[i]].push_back(id);
        }
        tspkg.set_reason(reason);
        packages.emplace_back(std::move(tspkg));

        TransactionPackage old_tspkg(
            rpm::Package(sack, rpm::PackageId(obs[0])), TransactionPackage::Action::DOWNGRADED, reason);
        packages.emplace_back(std::move(old_tspkg));
    }
    auto list_reinstalls = solved_goal.list_reinstalls();
    for (auto index = 0; index < list_reinstalls.size(); ++index) {
        Id id = list_reinstalls[index];
        auto obs = solved_goal.list_obsoleted_by_package(id);
        if (obs.empty()) {
            throw RuntimeError("No obsoletes for reinstall");
        }
        rpm::Package new_package(sack, rpm::PackageId(id));
        auto reason = new_package.get_reason();
        TransactionPackage tspkg(new_package, TransactionPackage::Action::REINSTALL, reason);
        tspkg.replaces.emplace(rpm::Package(sack, rpm::PackageId(obs[0])));
        for (int i = 1; i < obs.size(); ++i) {
            rpm::Package obsoleted(sack, rpm::PackageId(obs[i]));
            auto obs_reson = obsoleted.get_reason();
            if (obs_reson > reason) {
                reason = obs_reson;
            }
            tspkg.obsoletes.emplace_back(std::move(obsoleted));
            obsoletes[obs[i]].push_back(id);
        }
        tspkg.set_reason(reason);
        packages.emplace_back(std::move(tspkg));

        TransactionPackage old_tspkg(
            rpm::Package(sack, rpm::PackageId(obs[0])), TransactionPackage::Action::REINSTALLED, reason);
        packages.emplace_back(std::move(old_tspkg));
    }
    auto list_installs = solved_goal.list_installs();
    for (auto index = 0; index < list_installs.size(); ++index) {
        Id id = list_installs[index];
        auto obs = solved_goal.list_obsoleted_by_package(id);
        auto reason = solved_goal.get_reason(id);

        TransactionPackage tspkg(
            rpm::Package(sack, rpm::PackageId(id)), TransactionPackage::Action::INSTALL, reason);

        //  Inherit the reason if package is installonly an package with the same name is installed
        //  Use the same logic like upgrade
        //  Upgrade of installonly packages result in install or install and remove step
        if (installonly_query.p_impl->contains(id)) {
            rpm::PackageQuery query(installed_installonly_query);
            query.filter_name({rpm::solv::get_name(pool, id)});
            if (!query.empty()) {
                reason = tspkg.get_package().get_reason();
            }
        }
        for (int i = 0; i < obs.size(); ++i) {
            rpm::Package obsoleted(sack, rpm::PackageId(obs[i]));
            auto obs_reson = obsoleted.get_reason();
            if (obs_reson > reason) {
                reason = obs_reson;
            }
            tspkg.obsoletes.emplace_back(std::move(obsoleted));
            obsoletes[obs[i]].push_back(id);
        }
        tspkg.set_reason(reason);
        packages.emplace_back(std::move(tspkg));
    }
    auto list_upgrades = solved_goal.list_upgrades();
    for (auto index = 0; index < list_upgrades.size(); ++index) {
        Id id = list_upgrades[index];
        auto obs = solved_goal.list_obsoleted_by_package(id);
        if (obs.empty()) {
            throw RuntimeError("No obsoletes for reinstall");
        }
        rpm::Package new_package(sack, rpm::PackageId(id));
        auto reason = new_package.get_reason();
        TransactionPackage tspkg(new_package, TransactionPackage::Action::UPGRADE, reason);
        tspkg.replaces.emplace(rpm::Package(sack, rpm::PackageId(obs[0])));
        for (int i = 1; i < obs.size(); ++i) {
            rpm::Package obsoleted(sack, rpm::PackageId(obs[i]));
            auto obs_reson = obsoleted.get_reason();
            if (obs_reson > reason) {
                reason = obs_reson;
            }
            tspkg.obsoletes.emplace_back(std::move(obsoleted));
            obsoletes[obs[i]].push_back(id);
        }
        tspkg.set_reason(reason);
        packages.emplace_back(std::move(tspkg));

        TransactionPackage old_tspkg(
            rpm::Package(sack, rpm::PackageId(obs[0])), TransactionPackage::Action::UPGRADED, reason);
        packages.emplace_back(std::move(old_tspkg));
    }
    auto list_removes = solved_goal.list_removes();
    if (!list_removes.empty()) {
        rpm::PackageQuery remaining_installed(base, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
        remaining_installed.filter_installed();
        for (auto index = 0; index < list_removes.size(); ++index) {
            remaining_installed.p_impl->remove(list_removes[index]);
        }
        rpm::PackageSet tmp_set(base->get_rpm_package_sack());

        // https://bugzilla.redhat.com/show_bug.cgi?id=1921063
        // To keep a reason of installonly pkgs in DB for remove step it requires TSI with reason change
        for (auto index = 0; index < list_removes.size(); ++index) {
            Id id = list_removes[index];
            rpm::Package rm_package(sack, rpm::PackageId(id));
            tmp_set.add(rm_package);
            rpm::PackageQuery remaining_na(remaining_installed);
            remaining_na.filter_name_arch(tmp_set);
            if (!remaining_na.empty()) {
                auto keep_reason = (*remaining_na.begin()).get_reason();
                TransactionPackage keep_reason_tspkg(
                    *remaining_na.begin(), TransactionPackage::Action::REASON_CHANGE, keep_reason);
                packages.emplace_back(std::move(keep_reason_tspkg));
            }
            tmp_set.clear();
            auto reason = solved_goal.get_reason(id);
            TransactionPackage tspkg(rm_package, TransactionPackage::Action::REMOVE, reason);
            packages.emplace_back(std::move(tspkg));
        }
    }

    // Add obsoleted packages
    for (const auto & [obsoleted_id, obsoleted_by_ids] : obsoletes) {
        rpm::Package obsoleted(sack, rpm::PackageId(obsoleted_id));
        auto reason = solved_goal.get_reason(obsoleted_id);
        TransactionPackage tsi(obsoleted, TransactionPackage::Action::OBSOLETED, reason);
        for (auto id : obsoleted_by_ids) {
            tsi.obsoletes.emplace_back(rpm::Package(sack, rpm::PackageId(id)));
        }
    }
}


}  // namespace libdnf::base
