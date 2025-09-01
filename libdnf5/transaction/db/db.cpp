// Copyright Contributors to the DNF5 project.
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


#include "db.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <filesystem>


namespace libdnf5::transaction {


static constexpr const char * SQL_CREATE_TABLES =
#include "sql/create_tables.sql"
    ;


static constexpr const char * SQL_TABLE_CONFIG_EXISTS = R"**(
    SELECT
        "name"
    FROM
        "sqlite_master"
    WHERE
        "type" = 'table'
        AND "name" = 'config'
)**";


static constexpr const char * SQL_GET_SCHEMA_VERSION = R"**(
    SELECT
        "value"
    FROM
        "config"
    WHERE
        "key" = 'version'
)**";


// Create tables and migrate schema if necessary.
static void transaction_db_create(libdnf5::utils::SQLite3 & conn) {
    // check if table 'config' exists; if not, assume an empty database and create the tables
    libdnf5::utils::SQLite3::Statement query_table_config_exists(conn, SQL_TABLE_CONFIG_EXISTS);
    if (query_table_config_exists.step() != libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        conn.exec(SQL_CREATE_TABLES);
    }

    std::string schema_version;
    libdnf5::utils::SQLite3::Statement query_get_schema_version(conn, SQL_GET_SCHEMA_VERSION);
    if (query_get_schema_version.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        schema_version = query_get_schema_version.get<std::string>(0);
    } else {
        throw RuntimeError(M_("Unable to get 'version' from table 'config'"));
    }

    // TODO(dmach): migrations
}


std::unique_ptr<libdnf5::utils::SQLite3> transaction_db_connect(libdnf5::Base & base) {
    auto & config = base.get_config();
    config.get_installroot_option().lock("installroot locked by transaction_db_connect");

    std::filesystem::path path{config.get_installroot_option().get_value()};
    path /= std::filesystem::path(config.get_transaction_history_dir_option().get_value()).relative_path();
    std::filesystem::create_directories(path);

    auto conn = std::make_unique<libdnf5::utils::SQLite3>((path / "transaction_history.sqlite").native());
    transaction_db_create(*conn);
    return conn;
}


}  // namespace libdnf5::transaction
