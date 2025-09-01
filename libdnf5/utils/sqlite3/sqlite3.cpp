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


#include "sqlite3.hpp"

#include "libdnf5/utils/bgettext/bgettext-lib.h"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"


namespace libdnf5::utils {

const BgettextMessage SQLite3::Statement::msg_compilation_failed = M_("SQL statement compilation failed: \"{}\"");
const BgettextMessage SQLite3::Statement::msg_bind_int_failed =
    M_("Binding integer value to SQL statement failed: \"{}\"");
const BgettextMessage SQLite3::Statement::msg_bind_int64_failed =
    M_("Binding integer64 value to SQL statement failed: \"{}\"");
const BgettextMessage SQLite3::Statement::msg_bind_uint32_failed =
    M_("Binding unsigned integer32 value to SQL statement failed: \"{}\"");
const BgettextMessage SQLite3::Statement::msg_bind_double_failed =
    M_("Binding double value to SQL statement failed: \"{}\"");
const BgettextMessage SQLite3::Statement::msg_bind_bool_failed =
    M_("Binding bool value to SQL statement failed: \"{}\"");
const BgettextMessage SQLite3::Statement::msg_bind_text_failed = M_("Binding text to SQL statement failed: \"{}\"");
const BgettextMessage SQLite3::Statement::msg_bind_blob_failed = M_("Binding blob to SQL statement failed: \"{}\"");
const BgettextMessage SQLite3::Statement::msg_eval_failed = M_("SQL statement evaluation failed: \"{}\"");
const BgettextMessage SQLite3::Statement::msg_insufficient_memory =
    M_("Insufficient memory or result exceed maximum SQLite3 string length");

const BgettextMessage SQLite3::Query::msg_column_not_found = M_("Column \"{}\" not found");

const BgettextMessage SQLite3::msg_statement_exec_failed = M_("SQL statement execution failed: \"{}\"");

const char * SQLite3SQLError::SQLite3SQLError::what() const noexcept {
    try {
        message = fmt::format(
            "{}: ({}) - {}",
            formatter ? formatter(TM_(format, 1)) : TM_(format, 1),
            error_code,
            sqlite3_errstr(error_code));
    } catch (...) {
        return TM_(format, 1);
    }
    return message.c_str();
}

void SQLite3::open() {
    if (db == nullptr) {
        auto result = sqlite3_open(path.c_str(), &db);
        if (result != SQLITE_OK) {
            sqlite3_close(db);
            throw SQLite3SQLError(result, M_("Failed to open database \"{}\""), path);
        }

        // the busy timeout must be set before executing *any* statements
        // because even setting PRAGMAs can fail with "database is locked" error
        sqlite3_busy_timeout(db, 10000);

#if SQLITE_VERSION_NUMBER >= 3022000
        int enabled = 1;
        sqlite3_file_control(db, "main", SQLITE_FCNTL_PERSIST_WAL, &enabled);
        if (sqlite3_db_readonly(db, "main") == 1) {
            exec("PRAGMA locking_mode = NORMAL; PRAGMA foreign_keys = ON;");
        } else {
            exec("PRAGMA locking_mode = NORMAL; PRAGMA journal_mode = WAL; PRAGMA foreign_keys = ON;");
        }
#else
        // Journal mode WAL in readonly mode is supported from sqlite version 3.22.0
        exec("PRAGMA locking_mode = NORMAL; PRAGMA journal_mode = TRUNCATE; PRAGMA foreign_keys = ON;");
#endif
    }
}


void SQLite3::close() {
    if (db == nullptr) {
        return;
    }

    auto result = sqlite3_close(db);
    if (result == SQLITE_BUSY) {
        sqlite3_stmt * res = nullptr;
        while ((res = sqlite3_next_stmt(db, nullptr))) {
            sqlite3_finalize(res);
        }
        result = sqlite3_close(db);
    }
    if (result != SQLITE_OK) {
        throw SQLite3SQLError(result, M_("Failed to close database \"{}\""), path);
    }
    db = nullptr;
}


void SQLite3::backup(const std::string & output_file) {
    sqlite3 * backup_db = nullptr;

    auto result = sqlite3_open(output_file.c_str(), &backup_db);
    if (result != SQLITE_OK) {
        sqlite3_close(backup_db);
        throw SQLite3SQLError(result, M_("Failed to open backup database \"{}\""), output_file);
    }

    sqlite3_backup * backup_handle = sqlite3_backup_init(backup_db, "main", db, "main");

    if (backup_handle) {
        sqlite3_backup_step(backup_handle, -1);
        sqlite3_backup_finish(backup_handle);
    }

    result = sqlite3_errcode(backup_db);

    sqlite3_close(backup_db);

    if (result != SQLITE_OK) {
        throw SQLite3SQLError(result, M_("Failed to backup database \"{}\" into \"{}\""), path, output_file);
    }
}


void SQLite3::restore(const std::string & input_file) {
    sqlite3 * backup_db = nullptr;

    auto result = sqlite3_open(input_file.c_str(), &backup_db);
    if (result != SQLITE_OK) {
        sqlite3_close(backup_db);
        throw SQLite3SQLError(result, M_("Failed to open backup database \"{}\""), input_file);
    }

    sqlite3_backup * backup_handle = sqlite3_backup_init(db, "main", backup_db, "main");

    if (backup_handle) {
        sqlite3_backup_step(backup_handle, -1);
        sqlite3_backup_finish(backup_handle);
    }

    result = sqlite3_errcode(backup_db);

    sqlite3_close(backup_db);

    if (result != SQLITE_OK) {
        throw SQLite3SQLError(result, M_("Failed to restore database \"{}\""), input_file);
    }
}


}  // namespace libdnf5::utils
