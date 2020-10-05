#pragma once


#include "libdnf/utils/sqlite3/sqlite3.hpp"


namespace libdnf::transaction {


static const char * SQL_CREATE_TABLES =
#include "sql/create_tables.sql"
    ;



inline void create_database(libdnf::utils::SQLite3 & conn) {
    conn.exec(SQL_CREATE_TABLES);
}


}  // libdnf::transaction
