// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "item.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"


namespace libdnf5::transaction {


static constexpr const char * SQL_ITEM_INSERT = R"**(
    INSERT INTO "item" DEFAULT VALUES
)**";


std::unique_ptr<libdnf5::utils::SQLite3::Statement> item_insert_new_query(libdnf5::utils::SQLite3 & conn) {
    auto query = std::make_unique<libdnf5::utils::SQLite3::Statement>(conn, SQL_ITEM_INSERT);
    return query;
}


int64_t item_insert(libdnf5::utils::SQLite3::Statement & query) {
    if (query.step() != libdnf5::utils::SQLite3::Statement::StepResult::DONE) {
        // TODO(dmach): replace with a better exception class
        throw RuntimeError(M_("Failed to insert record into table 'item' in history database"));
    }
    return query.last_insert_rowid();
}


}  // namespace libdnf5::transaction
