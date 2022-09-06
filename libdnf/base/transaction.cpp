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


#include "rpm/transaction.hpp"

#include "base_impl.hpp"
#include "rpm/package_set_impl.hpp"
#include "solv/pool.hpp"
#include "solver_problems_internal.hpp"
#include "transaction_impl.hpp"
#include "utils/bgettext/bgettext-lib.h"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/locker.hpp"
#include "utils/string.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/common/proc.hpp"
#include "libdnf/rpm/package_query.hpp"

#include <fmt/format.h>

#include <filesystem>
#include <iostream>
#include <ranges>


namespace libdnf::base {

namespace {

// Maps the string representation of transaction flags to the rpmtransFlags_e enum.
constexpr std::pair<const char *, rpmtransFlags_e> string_tsflag_map[]{
    {"test", RPMTRANS_FLAG_TEST},
    {"nodocs", RPMTRANS_FLAG_NODOCS},
    {"noscripts", RPMTRANS_FLAG_NOSCRIPTS},
    {"notriggers", RPMTRANS_FLAG_NOTRIGGERS},
    {"justdb", RPMTRANS_FLAG_JUSTDB},
    {"nocontexts", RPMTRANS_FLAG_NOCONTEXTS},
    {"nocaps", RPMTRANS_FLAG_NOCAPS},
    {"nocrypto", RPMTRANS_FLAG_NOFILEDIGEST},
};

const std::map<base::Transaction::TransactionRunResult, BgettextMessage> TRANSACTION_RUN_RESULT_DICT = {
    {base::Transaction::TransactionRunResult::ERROR_RERUN, M_("This transaction has been already run before.")},
    {base::Transaction::TransactionRunResult::ERROR_RESOLVE, M_("Cannot run transaction with resolving problems.")},
    {base::Transaction::TransactionRunResult::ERROR_CHECK, M_("Rpm transaction check failed.")},
    {base::Transaction::TransactionRunResult::ERROR_LOCK,
     M_("Failed to obtain rpm transaction lock. Another transaction is in progress.")},
    {base::Transaction::TransactionRunResult::ERROR_RPM_RUN, M_("Rpm transaction failed.")},
};

}  // namespace

Transaction::Transaction(const BaseWeakPtr & base) : p_impl(new Impl(*this, base)) {}
Transaction::Transaction(const Transaction & transaction) : p_impl(new Impl(*this, *transaction.p_impl)) {}

Transaction::Transaction(Transaction && transaction) : p_impl(std::move(transaction.p_impl)) {
    p_impl->transaction = this;
}

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

std::vector<TransactionPackage> Transaction::get_transaction_packages() const {
    return p_impl->packages;
}

std::vector<TransactionGroup> & Transaction::get_transaction_groups() const {
    return p_impl->groups;
}

GoalProblem Transaction::Impl::report_not_found(
    GoalAction action, const std::string & pkg_spec, const GoalJobSettings & settings, bool strict) {
    auto sack = base->get_rpm_package_sack();
    rpm::PackageQuery query(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
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
    resolve_logs.emplace_back(LogEvent(action, problem, settings, spec, additional_data));
    // TODO(jmracek) Use a logger properly
    auto & logger = *base->get_logger();
    if (strict) {
        logger.error(resolve_logs.back().to_string());
    } else {
        logger.warning(resolve_logs.back().to_string());
    }
}

void Transaction::Impl::add_resolve_log(
    GoalProblem problem, std::vector<std::vector<std::pair<libdnf::ProblemRules, std::vector<std::string>>>> problems) {
    resolve_logs.emplace_back(LogEvent(problem, problems));
    // TODO(jmracek) Use a logger properly
    auto & logger = *base->get_logger();
    logger.error(resolve_logs.back().to_string());
}

const std::vector<LogEvent> & Transaction::get_resolve_logs() const {
    return p_impl->resolve_logs;
}

std::vector<std::string> Transaction::get_resolve_logs_as_strings() const {
    std::vector<std::string> logs;
    for (const auto & log : get_resolve_logs()) {
        logs.emplace_back(log.to_string());
    }
    return logs;
}


std::string Transaction::transaction_result_to_string(const TransactionRunResult result) {
    switch (result) {
        case TransactionRunResult::SUCCESS:
            return {};
        case TransactionRunResult::ERROR_RERUN:
        case TransactionRunResult::ERROR_RESOLVE:
        case TransactionRunResult::ERROR_CHECK:
        case TransactionRunResult::ERROR_LOCK:
        case TransactionRunResult::ERROR_RPM_RUN:
            return TM_(TRANSACTION_RUN_RESULT_DICT.at(result), 1);
    }
    return {};
}

Transaction::TransactionRunResult Transaction::run(
    std::unique_ptr<libdnf::rpm::TransactionCallbacks> && callbacks,
    const std::string & description,
    const std::optional<uint32_t> user_id,
    const std::optional<std::string> comment) {
    return p_impl->run(std::move(callbacks), description, user_id, comment);
}

std::vector<std::string> Transaction::get_transaction_problems() const noexcept {
    return p_impl->transaction_problems;
}

void Transaction::Impl::set_transaction(rpm::solv::GoalPrivate & solved_goal, GoalProblem problems) {
    auto solver_problems = process_solver_problems(base, solved_goal);
    if (!solver_problems.empty()) {
        add_resolve_log(GoalProblem::SOLVER_ERROR, solver_problems);
    } else {
        // TODO(jmracek) To improve performance add a test whether it make sence to resolve transaction in strict mode
        // Test whether there were skipped jobs or used not the best candidates due to broken dependencies
        rpm::solv::GoalPrivate solved_goal_copy(solved_goal);
        solved_goal_copy.set_run_in_strict_mode(true);
        solved_goal_copy.resolve();
        auto solver_problems_strict = process_solver_problems(base, solved_goal_copy);
        if (!solver_problems_strict.empty()) {
            add_resolve_log(GoalProblem::SOLVER_PROBLEM_STRICT_RESOLVEMENT, solver_problems_strict);
        }
    }
    this->problems = problems;
    auto transaction = solved_goal.get_transaction();
    libsolv_transaction = transaction ? transaction_create_clone(transaction) : nullptr;
    if (!libsolv_transaction) {
        return;
    }

    // std::map<replaced, replaced_by>
    std::map<Id, std::vector<Id>> replaced;

    rpm::PackageQuery installed_query(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
    installed_query.filter_installed();

    // The order of packages in the vector matters, we rely on outbound actions
    // being at the end in Transaction::Impl::run()
    for (auto id : solved_goal.list_installs()) {
        packages.emplace_back(
            make_transaction_package(id, TransactionPackage::Action::INSTALL, solved_goal, replaced, installed_query));
    }

    for (auto id : solved_goal.list_reinstalls()) {
        packages.emplace_back(make_transaction_package(
            id, TransactionPackage::Action::REINSTALL, solved_goal, replaced, installed_query));
    }

    for (auto id : solved_goal.list_upgrades()) {
        packages.emplace_back(
            make_transaction_package(id, TransactionPackage::Action::UPGRADE, solved_goal, replaced, installed_query));
    }

    for (auto id : solved_goal.list_downgrades()) {
        packages.emplace_back(make_transaction_package(
            id, TransactionPackage::Action::DOWNGRADE, solved_goal, replaced, installed_query));
    }

    for (auto id : solved_goal.list_removes()) {
        packages.emplace_back(TransactionPackage(
            rpm::Package(base, rpm::PackageId(id)), TransactionPackage::Action::REMOVE, solved_goal.get_reason(id)));
    }

    // Add replaced packages to transaction
    for (const auto & [replaced_id, replaced_by_ids] : replaced) {
        rpm::Package obsoleted(base, rpm::PackageId(replaced_id));
        TransactionPackage tspkg(obsoleted, TransactionPackage::Action::REPLACED, obsoleted.get_reason());
        for (auto id : replaced_by_ids) {
            tspkg.replaced_by.emplace_back(rpm::Package(base, rpm::PackageId(id)));
        }
        packages.emplace_back(std::move(tspkg));
    }

    // Add groups to the transaction
    for (auto & [group, action, reason] : solved_goal.list_groups()) {
        groups.emplace_back(group, action, reason);
    }
}


TransactionPackage Transaction::Impl::make_transaction_package(
    Id id,
    TransactionPackage::Action action,
    rpm::solv::GoalPrivate & solved_goal,
    std::map<Id, std::vector<Id>> & replaced,
    rpm::PackageQuery installed_query) {
    auto obs = solved_goal.list_obsoleted_by_package(id);

    rpm::Package new_package(base, rpm::PackageId(id));

    transaction::TransactionItemReason reason;
    if (action == TransactionPackage::Action::INSTALL) {
        reason = std::max(new_package.get_reason(), solved_goal.get_reason(id));

        installed_query.filter_name({new_package.get_name()});
        installed_query.filter_arch({new_package.get_arch()});
        // For installonly packages: if the NA is already on the system, but
        // not recorded in system state as installed, it was installed outside
        // DNF and we want to preserve NONE as reason
        // TODO(lukash) this is still required even with having EXTERNAL_USER
        // as a reason, because with --best the reason returned from the goal
        // is USER, which is wrong here
        if (!installed_query.empty() && new_package.get_reason() == transaction::TransactionItemReason::EXTERNAL_USER) {
            reason = transaction::TransactionItemReason::EXTERNAL_USER;
        }
    } else {
        reason = new_package.get_reason();
    }

    TransactionPackage tspkg(new_package, action, reason);

    for (auto replaced_id : obs) {
        rpm::Package replaced_pkg(base, rpm::PackageId(replaced_id));
        reason = std::max(reason, replaced_pkg.get_reason());
        tspkg.replaces.emplace_back(std::move(replaced_pkg));
        replaced[replaced_id].push_back(id);
    }
    tspkg.set_reason(reason);

    return tspkg;
}

Transaction::TransactionRunResult Transaction::Impl::run(
    std::unique_ptr<libdnf::rpm::TransactionCallbacks> && callbacks,
    const std::string & description,
    const std::optional<uint32_t> user_id,
    const std::optional<std::string> comment) {
    // do not allow to run transaction multiple times
    if (history_db_id > 0) {
        return TransactionRunResult::ERROR_RERUN;
    }

    // only successfully resolved transaction can be run
    if (transaction->get_problems() != libdnf::GoalProblem::NO_PROBLEM) {
        return TransactionRunResult::ERROR_RESOLVE;
    }

    auto & config = base->get_config();

    // acquire the lock
    std::filesystem::path lock_file_path = config.installroot().get_value();
    lock_file_path /= "run/dnf/rpmtransaction.lock";
    std::filesystem::create_directories(lock_file_path.parent_path());

    libdnf::utils::Locker locker(lock_file_path);
    if (!locker.lock()) {
        return TransactionRunResult::ERROR_LOCK;
    }

    // fill and check the rpm transaction
    libdnf::rpm::Transaction rpm_transaction(base);
    rpm_transaction.fill(*transaction);
    if (!rpm_transaction.check()) {
        auto problems = rpm_transaction.get_problems();
        for (auto it = problems.begin(); it != problems.end(); ++it) {
            transaction_problems.emplace_back((*it).to_string());
        }
        return TransactionRunResult::ERROR_CHECK;
    }

    rpmtransFlags rpm_transaction_flags{RPMTRANS_FLAG_NONE};
    for (const auto & tsflag : config.tsflags().get_value()) {
        bool found = false;
        for (const auto & [string_name, enum_item] : string_tsflag_map) {
            if (tsflag == string_name) {
                rpm_transaction_flags |= static_cast<rpmtransFlags>(enum_item);
                found = true;
                break;
            }
        }
        if (!found) {
            throw TransactionError(M_("Invalid tsflag: {}"), tsflag);
        }

        // The "nocrypto" option will also set signature verify flags.
        if (tsflag == "nocrypto") {
            rpm_transaction.set_signature_verify_flags(
                rpm_transaction.get_signature_verify_flags() | RPMVSF_MASK_NOSIGNATURES | RPMVSF_MASK_NODIGESTS);
        }
    }

    // Run rpm transaction test
    rpm_transaction.set_flags(rpm_transaction_flags | RPMTRANS_FLAG_TEST);
    //TODO(jrohel): Do we want callbacks for transaction test?
    //rpm_transaction.set_callbacks(std::move(callbacks));
    auto ret = rpm_transaction.run();
    if (ret != 0) {
        auto problems = rpm_transaction.get_problems();
        for (auto it = problems.begin(); it != problems.end(); ++it) {
            transaction_problems.emplace_back((*it).to_string());
        }
        return TransactionRunResult::ERROR_RPM_RUN;
    }

    // With RPMTRANS_FLAG_TEST return just before anything is stored permanently
    if (rpm_transaction_flags & RPMTRANS_FLAG_TEST) {
        return TransactionRunResult::SUCCESS;
    }

    auto & plugins = base->get_plugins();
    plugins.pre_transaction(*transaction);

    // start history db transaction
    auto db_transaction = base->get_transaction_history()->new_transaction();
    // save history db transaction id
    history_db_id = db_transaction.get_id();

    auto vars = base->get_vars();
    if (vars->contains("releasever")) {
        db_transaction.set_releasever(vars->get_value("releasever"));
    }

    if (comment) {
        db_transaction.set_comment(comment.value());
    }

    db_transaction.set_description(description);

    if (user_id) {
        db_transaction.set_user_id(user_id.value());
    } else {
        db_transaction.set_user_id(get_login_uid());
    }
    //
    // TODO(jrohel): nevra of running dnf5?
    //db_transaction.add_runtime_package("dnf5");

    db_transaction.set_rpmdb_version_begin(rpm_transaction.get_db_cookie());
    db_transaction.fill_transaction_packages(packages);

    if (!groups.empty()) {
        // consider currently installed packages + inbound packages as installed for group members
        rpm::PackageQuery installed_query(base);
        installed_query.filter_installed();
        std::set<std::string> installed_names{};
        for (const auto & pkg : installed_query) {
            installed_names.emplace(pkg.get_name());
        }
        for (const auto & tspkg : packages) {
            if (transaction_item_action_is_inbound(tspkg.get_action())) {
                installed_names.emplace(tspkg.get_package().get_name());
            }
        }
        db_transaction.fill_transaction_groups(groups, installed_names);
    }

    auto time = std::chrono::system_clock::now().time_since_epoch();
    db_transaction.set_dt_start(std::chrono::duration_cast<std::chrono::seconds>(time).count());
    db_transaction.start();

    // execute rpm transaction
    //TODO(jrohel): Send scriptlet output to better place
    rpm_transaction.set_script_out_file("scriptlet.out");
    rpm_transaction.set_callbacks(std::move(callbacks));
    rpm_transaction.set_flags(rpm_transaction_flags);
    ret = rpm_transaction.run();

    // TODO(mblaha): Handle ret == -1 and ret > 0, fill problems list

    if (ret == 0) {
        // set the new system state
        auto & system_state = base->p_impl->get_system_state();

        rpm::PackageQuery installed_query(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
        installed_query.filter_installed();
        std::set<std::string> inbound_packages_reason_group{};

        // Iterate in reverse, inbound actions are first in the vector, we want to process outbound first
        for (auto it = packages.rbegin(); it != packages.rend(); ++it) {
            auto tspkg = *it;
            const auto & pkg = tspkg.get_package();
            if (transaction_item_action_is_inbound(tspkg.get_action())) {
                auto tspkg_reason = tspkg.get_reason();
                switch (tspkg_reason) {
                    case transaction::TransactionItemReason::DEPENDENCY:
                    case transaction::TransactionItemReason::WEAK_DEPENDENCY:
                    case transaction::TransactionItemReason::USER:
                    case transaction::TransactionItemReason::EXTERNAL_USER:
                        system_state.set_package_reason(pkg.get_na(), tspkg_reason);
                        break;
                    case transaction::TransactionItemReason::GROUP:
                        inbound_packages_reason_group.emplace(pkg.get_name());
                        break;
                    case transaction::TransactionItemReason::NONE:
                    case transaction::TransactionItemReason::CLEAN:
                        break;
                }
                system_state.set_package_from_repo(pkg.get_nevra(), tspkg.get_package().get_repo_id());
            } else if (transaction_item_action_is_outbound(tspkg.get_action())) {
                // Check if the NA is still installed. We do this check for all outbound actions.
                //
                // For REMOVE, if there's a package with the same NA still on the system,
                // it's an installonly package and we're keeping the reason.
                //
                // For REPLACED, we don't know if it was UPGRADE/DOWNGRADE/REINSTALL
                // (we're keeping the reason) or it's an obsolete (we're removing the reason)

                // We need to filter out packages that are being removed in the transaction
                // (the installed query still contains the packages before this transaction)
                installed_query.filter_nevra({pkg.get_nevra()}, libdnf::sack::QueryCmp::NEQ);

                rpm::PackageQuery query(installed_query);
                query.filter_name({pkg.get_name()});
                query.filter_arch({pkg.get_arch()});
                if (query.empty()) {
                    system_state.remove_package_na_state(pkg.get_na());
                }

                // for a REINSTALL, the remove needs to happen first, hence the reverse iteration of the for loop
                system_state.remove_package_nevra_state(pkg.get_nevra());
            } else if (tspkg.get_action() == TransactionPackage::Action::REASON_CHANGE) {
                system_state.set_package_reason(pkg.get_na(), tspkg.get_reason());
            }
        }

        // Set correct system state for groups in the transaction
        for (const auto & tsgroup : groups) {
            auto group = tsgroup.get_group();
            if (transaction_item_action_is_inbound(tsgroup.get_action())) {
                libdnf::system::GroupState state;
                state.userinstalled = tsgroup.get_reason() == transaction::TransactionItemReason::USER;
                // Remember packages installed by this group
                for (const auto & pkg : group.get_packages()) {
                    auto pkg_name = pkg.get_name();
                    if (inbound_packages_reason_group.find(pkg_name) != inbound_packages_reason_group.end()) {
                        // inbound package with reason GROUP from this transaction
                        state.packages.emplace_back(pkg_name);
                    } else {
                        // also group packages that were installed before this transaction
                        // system state consideres as installed by group
                        rpm::PackageQuery query(installed_query);
                        query.filter_name({pkg_name});
                        if (!query.empty()) {
                            state.packages.emplace_back(pkg_name);
                        }
                    }
                }
                system_state.set_group_state(group.get_groupid(), state);
            }
        }

        system_state.set_rpmdb_cookie(rpm_transaction.get_db_cookie());

        system_state.save();
    }

    // finish history db transaction
    time = std::chrono::system_clock::now().time_since_epoch();
    db_transaction.set_dt_end(std::chrono::duration_cast<std::chrono::seconds>(time).count());
    // TODO(jrohel): Also save the rpm db cookie to system state.
    //               Possibility to detect rpm database change without the need for a history database.
    db_transaction.set_rpmdb_version_end(rpm_transaction.get_db_cookie());
    db_transaction.finish(
        ret == 0 ? libdnf::transaction::TransactionState::OK : libdnf::transaction::TransactionState::ERROR);

    plugins.post_transaction(*transaction);

    if (ret == 0) {
        return TransactionRunResult::SUCCESS;
    } else {
        auto problems = rpm_transaction.get_problems();
        for (auto it = problems.begin(); it != problems.end(); ++it) {
            transaction_problems.emplace_back((*it).to_string());
        }
        return TransactionRunResult::ERROR_RPM_RUN;
    }
}


}  // namespace libdnf::base
