/*
Copyright (C) 2017-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "sqlite3.hpp"


namespace libdnf::utils {


void SQLite3::open() {
    if (db == nullptr) {
        auto result = sqlite3_open(path.c_str(), &db);
        if (result != SQLITE_OK) {
            sqlite3_close(db);
            throw Error(*this, result, "Open failed");
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
        throw Error(*this, result, "Close failed");
    }
    db = nullptr;
}


void SQLite3::backup(const std::string & output_file) {
    sqlite3 * backup_db = nullptr;

    auto result = sqlite3_open(output_file.c_str(), &backup_db);
    if (result != SQLITE_OK) {
        sqlite3_close(backup_db);
        throw Error(*this, result, "Failed to open backup database: \"" + output_file + "\"");
    }

    sqlite3_backup * backup_handle = sqlite3_backup_init(backup_db, "main", db, "main");

    if (backup_handle) {
        sqlite3_backup_step(backup_handle, -1);
        sqlite3_backup_finish(backup_handle);
    }

    result = sqlite3_errcode(backup_db);

    sqlite3_close(backup_db);

    if (result != SQLITE_OK) {
        throw Error(*this, result, "Database backup failed");
    }
}


void SQLite3::restore(const std::string & input_file) {
    sqlite3 * backup_db = nullptr;

    auto result = sqlite3_open(input_file.c_str(), &backup_db);
    if (result != SQLITE_OK) {
        sqlite3_close(backup_db);
        throw Error(*this, result, "Failed to open backup database: \"" + input_file + "\"");
    }

    sqlite3_backup * backup_handle = sqlite3_backup_init(db, "main", backup_db, "main");

    if (backup_handle) {
        sqlite3_backup_step(backup_handle, -1);
        sqlite3_backup_finish(backup_handle);
    }

    result = sqlite3_errcode(backup_db);

    sqlite3_close(backup_db);

    if (result != SQLITE_OK) {
        throw Error(*this, result, "Database restore failed");
    }
}


}  // namespace libdnf::utils
