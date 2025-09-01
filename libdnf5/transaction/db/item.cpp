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
