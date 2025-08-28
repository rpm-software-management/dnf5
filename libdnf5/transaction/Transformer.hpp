// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

// TODO(dmach): keep refactoring and deliver something that works with the new code base
// the whole file is disabled via the SKIP macro because it doesn't compile with the new code
#ifdef SKIP

#ifndef LIBDNF5_TRANSACTION_TRANSFORMER_HPP
#define LIBDNF5_TRANSACTION_TRANSFORMER_HPP

#include "TransformerTransaction.hpp"
#include "comps_environment.hpp"
#include "comps_group.hpp"
#include "rpm_package.hpp"
#include "transaction.hpp"
#include "transaction_item.hpp"
#include "utils/sqlite3/sqlite3.hpp"

#include <json.h>

#include <memory>
#include <vector>

namespace libdnf5::transaction {

/**
 * Class providing an interface to the database transformation
 */
class Transformer {
public:
    class Exception : public std::runtime_error {
    public:
        Exception(const std::string & msg) : runtime_error(msg) {}
        Exception(const char * msg) : runtime_error(msg) {}
    };

    Transformer(const std::string & inputDir, const std::string & outputFile);
    void transform();

    static void createDatabase(libdnf5::utils::SQLite3 & conn);

    static TransactionItemReason getReason(const std::string & reason);

protected:
    void transformTrans(libdnf5::utils::SQLite3 & swdb, libdnf5::utils::SQLite3 & history);

    void transformGroups(libdnf5::utils::SQLite3 & swdb);
    void processGroupPersistor(libdnf5::utils::SQLite3 & swdb, struct json_object * root);

private:
    void transformPackages(libdnf5::utils::SQLite3 & history, TransformerTransaction & trans);
    void transformOutput(libdnf5::utils::SQLite3 & history, TransformerTransaction & trans);
    void transformTransWith(libdnf5::utils::SQLite3 & history, TransformerTransaction & trans);
    std::shared_ptr<CompsGroup> processGroup(Transaction & trans, const char * groupId, struct json_object * group);
    std::shared_ptr<CompsEnvironment> processEnvironment(
        Transaction & trans, const char * envId, struct json_object * env);
    std::string historyPath();
    const std::string inputDir;
    const std::string outputFile;
    const std::string transformFile;
};

}  // namespace libdnf5::transaction

#endif

#endif
