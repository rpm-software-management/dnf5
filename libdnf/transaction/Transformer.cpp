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

// TODO(dmach): keep refactoring and deliver something that works with the new code base
// the whole file is disabled via the SKIP macro because it doesn't compile with the new code
#ifdef SKIP

#include "Transformer.hpp"

#include "Swdb.hpp"
#include "rpm_package.hpp"
#include "transaction.hpp"
#include "transaction_item.hpp"
#include "utils/bgettext/bgettext-lib.h"
#include "utils/string.hpp"

#include "libdnf/rpm/nevra.hpp"

#include <dirent.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace libdnf::transaction {

static const char * sql_create_tables =
#include "sql/create_tables.sql"
    ;

void Transformer::createDatabase(libdnf::utils::SQLite3 & conn) {
    conn.exec(sql_create_tables);
}

/**
 * Map of supported actions (originally states): string -> enum
 */
static const std::map<std::string, TransactionItemAction> actions = {
    {"Install", TransactionItemAction::INSTALL},
    {"True-Install", TransactionItemAction::INSTALL},
    {"Dep-Install", TransactionItemAction::INSTALL},
    {"Downgrade", TransactionItemAction::DOWNGRADE},
    {"Downgraded", TransactionItemAction::DOWNGRADED},
    {"Obsoleting", TransactionItemAction::OBSOLETE},
    {"Obsoleted", TransactionItemAction::OBSOLETED},
    {"Update", TransactionItemAction::UPGRADE},
    {"Updated", TransactionItemAction::UPGRADED},
    {"Erase", TransactionItemAction::REMOVE},
    {"Reinstall", TransactionItemAction::REINSTALL},
    {"Reinstalled", TransactionItemAction::REINSTALL}};

/**
 * Map of supported reasons: string -> enum
 */
static const std::map<std::string, TransactionItemReason> reasons = {
    {"dep", TransactionItemReason::DEPENDENCY},
    {"user", TransactionItemReason::USER},
    {"clean", TransactionItemReason::CLEAN},
    {"weak", TransactionItemReason::WEAK_DEPENDENCY},
    {"group", TransactionItemReason::GROUP}};

/**
 * Convert string reason into appropriate enumerated variant
 */
TransactionItemReason Transformer::getReason(const std::string & reason) {
    auto it = reasons.find(reason);
    if (it == reasons.end()) {
        return TransactionItemReason::UNKNOWN;
    }
    return it->second;
}

/**
 * Default constructor of the Transformer object
 * \param outputFile path to output SQLite3 database
 * \param inputDir directory to load data from (e.g. `/var/lib/dnf/`)
 */
Transformer::Transformer(const std::string & inputDir, const std::string & outputFile)
    : inputDir(inputDir),
      outputFile(outputFile) {}

/**
 * Perform the database transformation routine.
 * The database is transformed in-memory.
 * Final scheme is dumped into outputFile
 */
void Transformer::transform() {
    libdnf::utils::SQLite3 swdb(":memory:");

    if (std::filesystem::exists(outputFile.c_str())) {
        throw std::runtime_error("DB file already exists:" + outputFile);
    }

    // create directory path if necessary
    std::filesystem::create_directories(std::filesystem::path(outputFile).parent_path());

    // create a new database file
    createDatabase(swdb);

    // migrate history db if it exists
    try {
        // make a copy of source database to make creating indexes temporary
        libdnf::utils::SQLite3 history(":memory:");
        history.restore(historyPath().c_str());

        // create additional indexes in the source database to increase conversion speed
        history.exec("CREATE INDEX IF NOT EXISTS i_trans_cmdline_tid ON trans_cmdline(tid);");
        history.exec("CREATE INDEX IF NOT EXISTS i_trans_data_pkgs_tid ON trans_data_pkgs(tid);");
        history.exec("CREATE INDEX IF NOT EXISTS i_trans_script_stdout_tid ON trans_script_stdout(tid);");
        history.exec("CREATE INDEX IF NOT EXISTS i_trans_with_pkgs_tid_pkgtupid ON trans_with_pkgs(tid, pkgtupid);");

        // transform objects
        transformTrans(swdb, history);

        // transform groups
        transformGroups(swdb);
    } catch (Exception &) {
        // TODO: use a different (more specific) exception
    }

    // dump database to a file
    swdb.backup(outputFile);
}

/**
 * Transform transactions from the history database
 * \param swdb pointer to swdb SQLite3 object
 * \param swdb pointer to history database SQLite3 object
 */
void Transformer::transformTrans(libdnf::utils::SQLite3 & swdb, libdnf::utils::SQLite3 & history) {
    std::vector<std::shared_ptr<TransformerTransaction>> result;

    // we need to left join with trans_cmdline
    // there is no cmdline for certain transactions (e.g. 1)
    const char * trans_sql = R"**(
        SELECT
            tb.tid as id,
            tb.timestamp as dt_begin,
            tb.rpmdb_version rpmdb_version_begin,
            tb.loginuid as user_id,
            te.timestamp as dt_end,
            te.rpmdb_version as rpmdb_version_end,
            te.return_code as state,
            tc.cmdline as cmdline
        FROM
            trans_beg tb
            JOIN trans_end te using(tid)
            LEFT JOIN trans_cmdline tc using(tid)
        ORDER BY
            tb.tid
    )**";

    const char * releasever_sql = R"**(
        SELECT DISTINCT
            trans_data_pkgs.tid as tid,
            yumdb_val as releasever
        FROM
            trans_data_pkgs
        JOIN
            pkg_yumdb USING (pkgtupid)
        WHERE
            yumdb_key='releasever'
    )**";

    // get release version for all the transactions
    std::map<int64_t, std::string> releasever;
    libdnf::utils::SQLite3::Query releasever_query(history, releasever_sql);
    while (releasever_query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        std::string releaseVerStr = releasever_query.get<std::string>("releasever");
        releasever[releasever_query.get<int64_t>("tid")] = releaseVerStr;
    }

    // iterate over history transactions
    libdnf::utils::SQLite3::Query query(history, trans_sql);
    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        TransformerTransaction trans(swdb);
        trans.set_id(query.get<int>("id"));
        trans.set_dt_begin(query.get<int64_t>("dt_begin"));
        trans.set_dt_end(query.get<int64_t>("dt_end"));
        trans.set_rpmdb_version_begin(query.get<std::string>("rpmdb_version_begin"));
        trans.set_rpmdb_version_end(query.get<std::string>("rpmdb_version_end"));

        // set release version if available
        auto it = releasever.find(trans.get_id());
        if (it != releasever.end()) {
            trans.set_releasever(it->second);
        }

        trans.set_user_id(query.get<int>("user_id"));
        trans.set_cmdline(query.get<std::string>("cmdline"));

        TransactionState state = query.get<int>("state") == 0 ? TransactionState::DONE : TransactionState::ERROR;

        transformPackages(history, trans);
        transformTransWith(history, trans);

        trans.begin();

        transformOutput(history, trans);

        trans.finish(state);
    }
}

static void fillPackage(std::shared_ptr<Package> rpm, libdnf::utils::SQLite3::Query & query) {
    rpm->set_name(query.get<std::string>("name"));
    rpm->set_epoch(std::to_string(query.get<uint32_t>("epoch")));
    rpm->set_version(query.get<std::string>("version"));
    rpm->set_release(query.get<std::string>("release"));
    rpm->set_arch(query.get<std::string>("arch"));
    //rpm->save();
}

/**
 * Transform binding between a Transaction and packages, which performed the transaction.
 * \param swdb pointer to swdb SQLite3 object
 * \param swdb pointer to history database SQLite3 object
 */
void Transformer::transformTransWith(libdnf::utils::SQLite3 & history, TransformerTransaction & trans) {
    const char * sql = R"**(
        SELECT
            name,
            epoch,
            version,
            release,
            arch
        FROM
            trans_with_pkgs
            JOIN pkgtups using (pkgtupid)
        WHERE
            tid=?
    )**";

    // transform stdout
    libdnf::utils::SQLite3::Query query(history, sql);
    query.bindv(trans.get_id());
    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        // create RPM item object
        auto rpm = std::make_shared<Package>(trans);
        fillPackage(rpm, query);
        trans.add_runtime_package(libdnf::rpm::to_nevra_string(*rpm));
    }
}

/**
 * Transform transaction console outputs.
 * \param swdb pointer to history database SQLite3 object
 */
void Transformer::transformOutput(libdnf::utils::SQLite3 & history, TransformerTransaction & trans) {
    const char * sql = R"**(
        SELECT
            line
        FROM
            trans_script_stdout
        WHERE
            tid = ?
        ORDER BY
            lid
    )**";

    // transform stdout
    libdnf::utils::SQLite3::Query query(history, sql);
    query.bindv(trans.get_id());
    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        trans.add_console_output_line(1, query.get<std::string>("line"));
    }

    sql = R"**(
        SELECT
            msg
        FROM
            trans_error
        WHERE
            tid = ?
        ORDER BY
            mid
    )**";

    // transform stderr
    libdnf::utils::SQLite3::Query errorQuery(history, sql);
    errorQuery.bindv(trans.get_id());
    while (errorQuery.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        trans.add_console_output_line(2, errorQuery.get<std::string>("msg"));
    }
}

static void getYumdbData(
    int64_t itemId, libdnf::utils::SQLite3 & history, TransactionItemReason & reason, std::string & repoid) {
    const char * sql = R"**(
        SELECT
            yumdb_key as key,
            yumdb_val as value
        FROM
            pkg_yumdb
        WHERE
            pkgtupid=?
            and key IN ('reason', 'from_repo')
    )**";

    // load reason and repoid data from yumdb
    libdnf::utils::SQLite3::Query query(history, sql);
    query.bindv(itemId);
    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        std::string key = query.get<std::string>("key");
        if (key == "reason") {
            reason = Transformer::getReason(query.get<std::string>("value"));
        } else if (key == "from_repo") {
            repoid = query.get<std::string>("value");
        }
    }
}

/**
 * Transform RPM Items from a particular transaction.
 * \param swdb pointer to swdb SQLite3 object
 * \param swdb pointer to history database SQLite3 objects
 * \param trans Transaction whose items should be transformed
 */
void Transformer::transformPackages(libdnf::utils::SQLite3 & history, TransformerTransaction & trans) {
    // the order is important here - its Update, Updated
    const char * pkg_sql = R"**(
        SELECT
            t.state,
            t.done,
            r.pkgtupid as id,
            r.name,
            r.epoch,
            r.version,
            r.release,
            r.arch
        FROM
            trans_data_pkgs t
            JOIN pkgtups r using(pkgtupid)
        WHERE
            t.tid=?
    )**";

    libdnf::utils::SQLite3::Query query(history, pkg_sql);
    query.bindv(trans.get_id());

    TransactionItemPtr last = nullptr;

    /*
     * Item in a single transaction can be both Obsoleted multiple times and Updated.
     * We need to keep track of all the obsoleted items,
     * so we can promote them to Updated in case.
     * Obsoleted records will be kept in item_replaced table,
     * so it's always obvious, that particular package was both Obsoleted
     * and Updated. Technically, we could replace action Obsoleted with action Erase.
     */
    std::map<int64_t, TransactionItemPtr> obsoletedItems;

    // iterate over transaction packages in the history database
    while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        // create RPM item object
        auto rpm = std::make_shared<Package>(trans);
        fillPackage(rpm, query);

        // get item state/action
        std::string stateString = query.get<std::string>("state");
        TransactionItemAction action = actions.at(stateString);

        // `Obsoleting` record is duplicated with previous record (with different action)
        if (action == TransactionItemAction::OBSOLETE) {
            continue;
        }

        // find out if an item was previously obsoleted
        auto pastObsoleted = obsoletedItems.find(rpm->get_id());

        TransactionItemPtr transItem = nullptr;

        if (pastObsoleted == obsoletedItems.end()) {
            // item hasn't been obsoleted yet

            // load reason and from_repo
            TransactionItemReason reason = TransactionItemReason::UNKNOWN;
            std::string repoid;
            getYumdbData(query.get<int64_t>("id"), history, reason, repoid);

            // add TransactionItem object
            transItem = trans.addItem(rpm, repoid, action, reason);
            transItem->set_state(
                query.get<std::string>("done") == "TRUE" ? TransactionItemState::DONE : TransactionItemState::ERROR);
        } else {
            // item has been obsoleted - we just need to update the action
            transItem = pastObsoleted->second;
            transItem->set_action(action);
        }

        // resolve replaced by
        switch (action) {
            case TransactionItemAction::OBSOLETED:
                obsoletedItems[rpm->get_id()] = transItem;
                //transItem->addReplacedBy(last);
                break;
            case TransactionItemAction::DOWNGRADED:
            case TransactionItemAction::UPGRADED:
                //transItem->addReplacedBy(last);
                break;
            default:
                break;
        }

        // keep the last item in case of obsoletes
        last = transItem;
    }
}

/**
 * Construct CompsGroup object from JSON
 * \param group group json object
 */
std::shared_ptr<CompsGroup> Transformer::processGroup(
    Transaction & trans, const char * groupId, struct json_object * group) {
    struct json_object * value;

    // create group
    auto compsGroup = std::make_shared<CompsGroup>(trans);

    compsGroup->set_group_id(groupId);

    if (json_object_object_get_ex(group, "name", &value)) {
        compsGroup->set_name(json_object_get_string(value));
    }

    if (json_object_object_get_ex(group, "ui_name", &value)) {
        compsGroup->set_translated_name(json_object_get_string(value));
    }

    // TODO parse pkg_types to PackageType
    if (json_object_object_get_ex(group, "full_list", &value)) {
        auto len = json_object_array_length(value);
        for (std::size_t i = 0; i < len; ++i) {
            const char * key = json_object_get_string(json_object_array_get_idx(value, i));
            auto & pkg = compsGroup->new_package();
            pkg.set_name(key);
            pkg.set_installed(true);
            pkg.set_package_type(PackageType::MANDATORY);
        }
    }

    // TODO parse pkg_types to PackageType
    if (json_object_object_get_ex(group, "pkg_exclude", &value)) {
        auto len = json_object_array_length(value);
        for (std::size_t i = 0; i < len; ++i) {
            const char * key = json_object_get_string(json_object_array_get_idx(value, i));
            auto & pkg = compsGroup->new_package();
            pkg.set_name(key);
            pkg.set_installed(false);
            pkg.set_package_type(PackageType::MANDATORY);
        }
    }

    //compsGroup->save();
    return compsGroup;
}

/**
 * Construct CompsEnvironmentItem object from JSON
 * \param env environment json object
 */
std::shared_ptr<CompsEnvironment> Transformer::processEnvironment(
    Transaction & trans, const char * envId, struct json_object * env) {
    struct json_object * value;

    // create environment
    auto compsEnv = std::make_shared<CompsEnvironment>(trans);
    compsEnv->set_environment_id(envId);

    if (json_object_object_get_ex(env, "name", &value)) {
        compsEnv->set_name(json_object_get_string(value));
    }

    if (json_object_object_get_ex(env, "ui_name", &value)) {
        compsEnv->set_translated_name(json_object_get_string(value));
    }

    // TODO parse pkg_types/grp_types to PackageType
    if (json_object_object_get_ex(env, "full_list", &value)) {
        auto len = json_object_array_length(value);
        for (std::size_t i = 0; i < len; ++i) {
            const char * key = json_object_get_string(json_object_array_get_idx(value, i));
            auto & grp = compsEnv->new_group();
            grp.set_group_id(key);
            grp.set_installed(true);
            grp.set_group_type(PackageType::MANDATORY);
        }
    }

    // TODO parse pkg_types/grp_types to PackageType
    if (json_object_object_get_ex(env, "pkg_exclude", &value)) {
        auto len = json_object_array_length(value);
        for (std::size_t i = 0; i < len; ++i) {
            const char * key = json_object_get_string(json_object_array_get_idx(value, i));
            auto & grp = compsEnv->new_group();
            grp.set_group_id(key);
            grp.set_installed(false);
            grp.set_group_type(PackageType::MANDATORY);
        }
    }

    //compsEnv->save();

    return compsEnv;
}

/**
 * Create fake transaction for groups in persistor
 * \param swdb pointer to swdb SQLite3 object
 * \param root group persistor root node
 */
void Transformer::processGroupPersistor(libdnf::utils::SQLite3 & swdb, struct json_object * root) {
    // there is no rpmdb change in this transaction,
    // use rpmdb version from the last converted transaction
    Swdb swdbObj(swdb, false);
    auto lastTrans = swdbObj.getLastTransaction();

    auto trans = Transaction(swdb);

    // load sequences
    struct json_object * groups;
    struct json_object * envs;

    // add groups
    if (json_object_object_get_ex(root, "GROUPS", &groups)) {
        json_object_object_foreach(groups, key, val) {
            trans.addItem(
                processGroup(trans, key, val),
                {},  // repoid
                TransactionItemAction::INSTALL,
                TransactionItemReason::USER);
        }
    }

    // add environments
    if (json_object_object_get_ex(root, "ENVIRONMENTS", &envs)) {
        json_object_object_foreach(envs, key, val) {
            trans.addItem(
                processEnvironment(trans, key, val),
                {},  // repoid
                TransactionItemAction::INSTALL,
                TransactionItemReason::USER);
        }
    }

    trans.begin();

    auto now = time(NULL);
    trans.set_dt_begin(now);
    trans.set_dt_end(now);

    if (lastTrans) {
        trans.set_rpmdb_version_begin(lastTrans->get_rpmdb_version_end());
        trans.set_rpmdb_version_end(trans.get_rpmdb_version_begin());
    } else {
        // no transaction found -> use 0 packages + hash for an empty string
        trans.set_rpmdb_version_begin("0:da39a3ee5e6b4b0d3255bfef95601890afd80709");
        trans.set_rpmdb_version_end(trans.get_rpmdb_version_begin());
    }

    /*
    for (auto i : trans.getItems()) {
        i->set_state(TransactionItemState::DONE);
        //i->save();
    }
    */

    trans.finish(TransactionState::DONE);
}

/**
 * Load group persistor into JSON object and perform transformation
 * \param swdb pointer to swdb SQLite3 object
 */
void Transformer::transformGroups(libdnf::utils::SQLite3 & swdb) {
    std::string groupsFile(inputDir);

    // create the groups.json path
    if (groupsFile.back() != '/') {
        groupsFile += '/';
    }
    groupsFile += "groups.json";

    std::ifstream groupsStream(groupsFile);

    if (!groupsStream.is_open()) {
        return;
    }

    std::stringstream buffer;
    buffer << groupsStream.rdbuf();

    struct json_object * root = json_tokener_parse(buffer.str().c_str());

    processGroupPersistor(swdb, root);
}

/**
 * Try to find the history database in the inputDir
 * \return path to the latest history database in the inputDir
 */
std::string Transformer::historyPath() {
    std::string historyDir(inputDir);

    // construct the history directory path
    if (historyDir.back() != '/') {
        historyDir += '/';
    }
    historyDir += "history";

    // vector for possible history DB files
    std::vector<std::string> possibleFiles;

    // open history directory
    struct dirent * dp;
    std::unique_ptr<DIR, std::function<void(DIR *)>> dirp(
        opendir(historyDir.c_str()), [](DIR * ptr) { closedir(ptr); });

    if (!dirp) {
        throw Exception(_("Transformer: can't open history persist dir"));
    }

    // iterate over history directory and look for 'history-*.sqlite' files
    while ((dp = readdir(dirp.get())) != nullptr) {
        std::string fileName(dp->d_name);
        if (libdnf::utils::string::starts_with(fileName, "history-") &&
            libdnf::utils::string::ends_with(fileName, ".sqlite")) {
            possibleFiles.push_back(fileName);
        }
    }

    if (possibleFiles.empty()) {
        throw Exception(_("Couldn't find a history database"));
    }

    // find the latest DB file
    std::sort(possibleFiles.begin(), possibleFiles.end());

    // return the path
    return historyDir + "/" + possibleFiles.back();
}

}  // namespace libdnf::transaction

#endif
