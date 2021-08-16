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


#include "transaction_impl.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/rpm/package_query.hpp"
#include "libdnf/rpm/package_set_impl.hpp"
#include "libdnf/solv/pool.hpp"
#include "libdnf/utils/bgettext/bgettext-lib.h"
#include "libdnf/utils/string.hpp"

#include <fmt/format.h>

#include <iostream>


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

GoalProblem Transaction::Impl::report_not_found(
    GoalAction action, const std::string & pkg_spec, const GoalJobSettings & settings, bool strict) {
    auto sack = base->get_rpm_package_sack();
    rpm::PackageQuery query(base, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
    if (action == GoalAction::REMOVE) {
        query.filter_installed();
    }
    auto nevra_pair_reports = query.resolve_pkg_spec(pkg_spec, settings, true);
    if (!nevra_pair_reports.first) {
        // RPM was not excluded or there is no related srpm
        add_resolve_log(action, GoalProblem::NOT_FOUND, settings, pkg_spec, {}, strict);
        if (settings.report_hint) {
            rpm::PackageQuery hints(base);
            if (action == GoalAction::REMOVE) {
                hints.filter_installed();
            }
            if (!settings.ignore_case && settings.with_nevra) {
                rpm::PackageQuery icase(hints);
                ResolveSpecSettings settings_copy = settings;
                settings_copy.ignore_case = true;
                settings_copy.with_provides = false;
                settings_copy.with_filenames = false;
                auto nevra_pair_icase = icase.resolve_pkg_spec(pkg_spec, settings_copy, false);
                if (nevra_pair_icase.first) {
                    add_resolve_log(
                        action, GoalProblem::HINT_ICASE, settings, pkg_spec, {(*icase.begin()).get_name()}, false);
                }
            }
            rpm::PackageQuery alternatives(hints);
            std::string alternatives_provide = fmt::format("alternative-for({})", pkg_spec);
            alternatives.filter_provides({alternatives_provide});
            if (!alternatives.empty()) {
                std::set<std::string> hints;
                for (auto pkg : alternatives) {
                    hints.emplace(pkg.get_name());
                }
                add_resolve_log(action, GoalProblem::HINT_ALTERNATIVES, settings, pkg_spec, hints, false);
            }
        }
        return GoalProblem::NOT_FOUND;
    }
    query.filter_repo_id({"src", "nosrc"}, sack::QueryCmp::NEQ);
    if (query.empty()) {
        add_resolve_log(action, GoalProblem::ONLY_SRC, settings, pkg_spec, {}, strict);
        return GoalProblem::ONLY_SRC;
    }
    // TODO(jmracek) make difference between regular excludes and modular excludes
    add_resolve_log(action, GoalProblem::EXCLUDED, settings, pkg_spec, {}, strict);
    return GoalProblem::EXCLUDED;
}

void Transaction::Impl::add_resolve_log(
    GoalAction action,
    GoalProblem problem,
    const GoalJobSettings & settings,
    const std::string & spec,
    const std::set<std::string> & additional_data,
    bool strict) {
    // TODO(jmracek) Use a logger properly and change a way how to report to terminal
    std::cout << Transaction::format_resolve_log(action, problem, settings, spec, additional_data) << std::endl;
    resolve_logs.emplace_back(std::make_tuple(action, problem, settings, spec, additional_data));
    auto & logger = *base->get_logger();
    if (strict) {
        logger.error(Transaction::format_resolve_log(action, problem, settings, spec, additional_data));
    } else {
        logger.warning(Transaction::format_resolve_log(action, problem, settings, spec, additional_data));
    }
}

std::string Transaction::format_resolve_log(
    GoalAction action,
    GoalProblem problem,
    const GoalJobSettings & settings,
    const std::string & spec,
    const std::set<std::string> & additional_data) {
    std::string ret;
    switch (problem) {
        // TODO(jmracek) Improve messages => Each message can contain also an action
        case GoalProblem::NOT_FOUND:
            if (action == GoalAction::REMOVE) {
                return ret.append(fmt::format(_("No packages to remove for argument: {}"), spec));
            }
            return ret.append(fmt::format(_("No match for argument: {}"), spec));
        case GoalProblem::NOT_FOUND_IN_REPOSITORIES:
            return ret.append(fmt::format(
                _("No match for argument '{0}' in repositories '{1}'"),
                spec,
                utils::string::join(settings.to_repo_ids, ", ")));
        case GoalProblem::NOT_INSTALLED:
            return ret.append(fmt::format(_("Packages for argument '{}' available, but not installed."), spec));
        case GoalProblem::NOT_INSTALLED_FOR_ARCHITECTURE:
            return ret.append(fmt::format(
                _("Packages for argument '{}' available, but installed for a different architecture."), spec));
        case GoalProblem::ONLY_SRC:
            return ret.append(fmt::format(_("Argument '{}' matches only source packages."), spec));
        case GoalProblem::EXCLUDED:
            return ret.append(fmt::format(_("Argument '{}' matches only excluded packages."), spec));
        case GoalProblem::HINT_ICASE:
            return ret.append(fmt::format(_("  * Maybe you meant: {}"), spec));
        case GoalProblem::HINT_ALTERNATIVES: {
            auto elements = utils::string::join(additional_data, ", ");
            return ret.append(fmt::format(_("There are following alternatives for '{0}': {1}"), spec, elements));
        }
        case GoalProblem::INSLALLED_LOWEST_VERSION: {
            if (additional_data.size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for INSLALLED_LOWEST_VERSION");
            }
            return ret.append(fmt::format(
                _("Package \"{}\" of lowest version already installed, cannot downgrade it."),
                *additional_data.begin()));
        }
        case GoalProblem::INSTALLED_IN_DIFFERENT_VERSION:
            if (action == GoalAction::REINSTALL) {
                return ret.append(fmt::format(
                    _("Packages for argument '{}' installed and available, but in a different version => cannot "
                      "reinstall"),
                    spec));
            }
            return ret.append(fmt::format(
                _("Packages for argument '{}' installed and available, but in a different version."), spec));
        case GoalProblem::NOT_AVAILABLE:
            return ret.append(fmt::format(_("Packages for argument '{}' installed, but not available."), spec));
        case GoalProblem::NO_PROBLEM:
        case GoalProblem::REMOVAL_OF_PROTECTED:
        case GoalProblem::SOLVER_ERROR:
            throw std::invalid_argument("Unsupported elements for a goal problem");
        case GoalProblem::ALREADY_INSLALLED:
            if (additional_data.size() != 1) {
                throw std::invalid_argument("Incorrect number of elements for ALREADY_INSLALLED");
            }
            return ret.append(fmt::format(_("Package \"{}\" is already installed."), *additional_data.begin()));
    }
    return ret;
}

const std::vector<
    std::tuple<libdnf::GoalAction, libdnf::GoalProblem, libdnf::GoalJobSettings, std::string, std::set<std::string>>> &
Transaction::get_resolve_logs() {
    return p_impl->resolve_logs;
}

void Transaction::Impl::set_transaction(rpm::solv::GoalPrivate & solved_goal, GoalProblem problems) {
    this->problems = problems;
    auto transaction = solved_goal.get_transaction();
    libsolv_transaction = transaction ? transaction_create_clone(transaction) : nullptr;
    if (!libsolv_transaction) {
        return;
    }
    auto & pool = get_pool(base);

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
        rpm::Package new_package(base, rpm::PackageId(id));
        auto reason = new_package.get_reason();
        TransactionPackage tspkg(new_package, TransactionPackage::Action::DOWNGRADE, reason);
        tspkg.replaces.emplace(rpm::Package(base, rpm::PackageId(obs[0])));
        for (int i = 1; i < obs.size(); ++i) {
            rpm::Package obsoleted(base, rpm::PackageId(obs[i]));
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
            rpm::Package(base, rpm::PackageId(obs[0])), TransactionPackage::Action::DOWNGRADED, reason);
        packages.emplace_back(std::move(old_tspkg));
    }
    auto list_reinstalls = solved_goal.list_reinstalls();
    for (auto index = 0; index < list_reinstalls.size(); ++index) {
        Id id = list_reinstalls[index];
        auto obs = solved_goal.list_obsoleted_by_package(id);
        if (obs.empty()) {
            throw RuntimeError("No obsoletes for reinstall");
        }
        rpm::Package new_package(base, rpm::PackageId(id));
        auto reason = new_package.get_reason();
        TransactionPackage tspkg(new_package, TransactionPackage::Action::REINSTALL, reason);
        tspkg.replaces.emplace(rpm::Package(base, rpm::PackageId(obs[0])));
        for (int i = 1; i < obs.size(); ++i) {
            rpm::Package obsoleted(base, rpm::PackageId(obs[i]));
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
            rpm::Package(base, rpm::PackageId(obs[0])), TransactionPackage::Action::REINSTALLED, reason);
        packages.emplace_back(std::move(old_tspkg));
    }
    auto list_installs = solved_goal.list_installs();
    for (auto index = 0; index < list_installs.size(); ++index) {
        Id id = list_installs[index];
        auto obs = solved_goal.list_obsoleted_by_package(id);
        auto reason = solved_goal.get_reason(id);

        TransactionPackage tspkg(rpm::Package(base, rpm::PackageId(id)), TransactionPackage::Action::INSTALL, reason);

        //  Inherit the reason if package is installonly an package with the same name is installed
        //  Use the same logic like upgrade
        //  Upgrade of installonly packages result in install or install and remove step
        if (installonly_query.p_impl->contains(id)) {
            rpm::PackageQuery query(installed_installonly_query);
            query.filter_name({pool.get_name(id)});
            if (!query.empty()) {
                reason = tspkg.get_package().get_reason();
            }
        }
        for (int i = 0; i < obs.size(); ++i) {
            rpm::Package obsoleted(base, rpm::PackageId(obs[i]));
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
        rpm::Package new_package(base, rpm::PackageId(id));
        auto reason = new_package.get_reason();
        TransactionPackage tspkg(new_package, TransactionPackage::Action::UPGRADE, reason);
        tspkg.replaces.emplace(rpm::Package(base, rpm::PackageId(obs[0])));
        for (int i = 1; i < obs.size(); ++i) {
            rpm::Package obsoleted(base, rpm::PackageId(obs[i]));
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
            rpm::Package(base, rpm::PackageId(obs[0])), TransactionPackage::Action::UPGRADED, reason);
        packages.emplace_back(std::move(old_tspkg));
    }
    auto list_removes = solved_goal.list_removes();
    if (!list_removes.empty()) {
        rpm::PackageQuery remaining_installed(base, rpm::PackageQuery::InitFlags::IGNORE_EXCLUDES);
        remaining_installed.filter_installed();
        for (auto index = 0; index < list_removes.size(); ++index) {
            remaining_installed.p_impl->remove(list_removes[index]);
        }
        rpm::PackageSet tmp_set(base);

        // https://bugzilla.redhat.com/show_bug.cgi?id=1921063
        // To keep a reason of installonly pkgs in DB for remove step it requires TSI with reason change
        for (auto index = 0; index < list_removes.size(); ++index) {
            Id id = list_removes[index];
            rpm::Package rm_package(base, rpm::PackageId(id));
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
        rpm::Package obsoleted(base, rpm::PackageId(obsoleted_id));
        auto reason = solved_goal.get_reason(obsoleted_id);
        TransactionPackage tspkg(obsoleted, TransactionPackage::Action::OBSOLETED, reason);
        for (auto id : obsoleted_by_ids) {
            tspkg.replaced_by.emplace_back(rpm::Package(base, rpm::PackageId(id)));
        }
        packages.emplace_back(std::move(tspkg));
    }
}


}  // namespace libdnf::base
