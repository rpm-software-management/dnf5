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
    }
    this->problems = problems;
    auto transaction = solved_goal.get_transaction();
    libsolv_transaction = transaction ? transaction_create_clone(transaction) : nullptr;
    if (!libsolv_transaction) {
        return;
    }
    auto & pool = get_pool(base);

    rpm::PackageQuery installonly_query(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
    installonly_query.filter_provides(base->get_config().installonlypkgs().get_value());
    rpm::PackageQuery installed_installonly_query(installonly_query);
    installed_installonly_query.filter_installed();

    // std::map<replaced, replaced_by>
    std::map<Id, std::vector<Id>> replaced;

    for (auto id : solved_goal.list_installs()) {
        // TODO(lukash) use make_transaction_package(), the installonly query makes it awkward
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
            tspkg.replaces.emplace_back(std::move(obsoleted));
            replaced[obs[i]].push_back(id);
        }
        tspkg.set_reason(reason);
        packages.emplace_back(std::move(tspkg));
    }

    for (auto id : solved_goal.list_reinstalls()) {
        packages.emplace_back(
            make_transaction_package(id, TransactionPackage::Action::REINSTALL, solved_goal, replaced));
    }

    for (auto id : solved_goal.list_upgrades()) {
        packages.emplace_back(make_transaction_package(id, TransactionPackage::Action::UPGRADE, solved_goal, replaced));
    }

    for (auto id : solved_goal.list_downgrades()) {
        packages.emplace_back(
            make_transaction_package(id, TransactionPackage::Action::DOWNGRADE, solved_goal, replaced));
    }

    auto list_removes = solved_goal.list_removes();
    if (!list_removes.empty()) {
        rpm::PackageQuery remaining_installed(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
        remaining_installed.filter_installed();
        for (auto id : list_removes) {
            remaining_installed.p_impl->remove(id);
        }
        rpm::PackageSet tmp_set(base);

        // https://bugzilla.redhat.com/show_bug.cgi?id=1921063
        // To keep a reason of installonly pkgs in DB for remove step it requires TSI with reason change
        for (auto id : list_removes) {
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

    // Add replaced packages to transaction
    for (const auto & [replaced_id, replaced_by_ids] : replaced) {
        rpm::Package obsoleted(base, rpm::PackageId(replaced_id));
        auto reason = solved_goal.get_reason(replaced_id);
        TransactionPackage tspkg(obsoleted, TransactionPackage::Action::REPLACED, reason);
        for (auto id : replaced_by_ids) {
            tspkg.replaced_by.emplace_back(rpm::Package(base, rpm::PackageId(id)));
        }
        packages.emplace_back(std::move(tspkg));
    }
}


TransactionPackage Transaction::Impl::make_transaction_package(
    Id id,
    TransactionPackage::Action action,
    rpm::solv::GoalPrivate & solved_goal,
    std::map<Id, std::vector<Id>> & replaced) {
    auto obs = solved_goal.list_obsoleted_by_package(id);

    libdnf_assert(!obs.empty(), "No obsoletes for {}", transaction_item_action_to_string(action));

    rpm::Package new_package(base, rpm::PackageId(id));
    auto reason = new_package.get_reason();
    TransactionPackage tspkg(new_package, action, reason);

    for (auto replaced_id : obs) {
        rpm::Package replaced_pkg(base, rpm::PackageId(replaced_id));
        auto old_reson = replaced_pkg.get_reason();
        if (old_reson > reason) {
            reason = old_reson;
        }
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

    db_transaction.set_cmdline(description);

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
        auto & system_state = base->get_system_state();
        for (const auto & tspkg : packages) {
            system_state.set_reason(tspkg.get_package().get_na(), tspkg.get_reason());
        }
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
