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

#ifndef LIBDNF5_UTILS_SQLITE3_SQLITE3_HPP
#define LIBDNF5_UTILS_SQLITE3_SQLITE3_HPP

#include "libdnf5/common/exception.hpp"

#include <sqlite3.h>

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


namespace libdnf5::utils {

class SQLite3Error : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::utils"; }
    const char * get_name() const noexcept override { return "SQLite3Error"; }
};

class SQLite3SQLError : public SQLite3Error {
public:
    /// @param error_code The `error_code` of the error.
    /// @param format The format string for the message.
    /// @param args The format arguments.
    template <typename... Ss>
    explicit SQLite3SQLError(int error_code, BgettextMessage format, Ss &&... args)
        : SQLite3Error(format, std::forward<Ss>(args)...),
          error_code(error_code) {}

    const char * what() const noexcept override;

    const char * get_name() const noexcept override { return "SQLite3SQLError"; }

    /// @return The SQLite3 error code of the error.
    int get_error_code() const noexcept { return error_code; }

    /// @return The error message associated with the SQLite3 error code.
    const char * get_error_message() const noexcept { return sqlite3_errstr(error_code); }

protected:
    int error_code;
};


/// An error class that will log the SQL statement in its constructor.
class SQLite3StatementSQLError : public SQLite3SQLError {
public:
    using SQLite3SQLError::SQLite3SQLError;
    const char * get_name() const noexcept override { return "SQLite3StatementSQLError"; }
};


class SQLite3 {
public:
    struct Blob {
        size_t size;
        const void * data;
    };

    class Statement {
    public:
        enum class StepResult { DONE, ROW, BUSY };

        Statement(const Statement &) = delete;
        Statement & operator=(const Statement &) = delete;

        Statement(SQLite3 & db, const char * sql) : db(db) {
            auto result = sqlite3_prepare_v2(db.db, sql, -1, &stmt, nullptr);
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_compilation_failed, std::string(sql));
            }
        };

        Statement(SQLite3 & db, const std::string & sql) : db(db) {
            auto result = sqlite3_prepare_v2(db.db, sql.c_str(), static_cast<int>(sql.length()) + 1, &stmt, nullptr);
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_compilation_failed, sql);
            }
        };

        void bind(int pos, int val) {
            auto result = sqlite3_bind_int(stmt, pos, val);
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_bind_int_failed, get_expanded_sql());
            }
        }

        void bind(int pos, std::int64_t val) {
            auto result = sqlite3_bind_int64(stmt, pos, val);
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_bind_int64_failed, get_expanded_sql());
            }
        }

        void bind(int pos, std::uint32_t val) {
            // SQLite doesn't support storing unsigned ints, for 32 bit unsigned bind,
            // we can just use the 64 bit version, it all converts to a single
            // signed INTEGER type in the DB
            auto result = sqlite3_bind_int64(stmt, pos, static_cast<int64_t>(val));
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_bind_uint32_failed, get_expanded_sql());
            }
        }

        void bind(int pos, double val) {
            auto result = sqlite3_bind_double(stmt, pos, val);
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_bind_double_failed, get_expanded_sql());
            }
        }

        void bind(int pos, bool val) {
            auto result = sqlite3_bind_int(stmt, pos, val ? 1 : 0);
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_bind_bool_failed, get_expanded_sql());
            }
        }

        void bind(int pos, const char * val) {
            auto result = sqlite3_bind_text(stmt, pos, val, -1, SQLITE_TRANSIENT);
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_bind_text_failed, get_expanded_sql());
            }
        }

        void bind(int pos, const std::string & val) {
            auto result = sqlite3_bind_text(stmt, pos, val.c_str(), -1, SQLITE_TRANSIENT);
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_bind_text_failed, get_expanded_sql());
            }
        }

        void bind(int pos, const Blob & val) {
            auto result = sqlite3_bind_blob(stmt, pos, val.data, static_cast<int>(val.size), SQLITE_TRANSIENT);
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_bind_blob_failed, get_expanded_sql());
            }
        }

        void bind(int pos, const std::vector<unsigned char> & val) {
            auto result = sqlite3_bind_blob(stmt, pos, val.data(), static_cast<int>(val.size()), SQLITE_TRANSIENT);
            if (result != SQLITE_OK) {
                throw SQLite3StatementSQLError(result, msg_bind_blob_failed, get_expanded_sql());
            }
        }

        template <typename... Args>
        Statement & bindv(Args &&... args) {
            using Pass = int[];
            int pos{0};
            (void)Pass{(bind(++pos, args), 0)...};
            return *this;
        }

        StepResult step() {
            auto result = sqlite3_step(stmt);
            switch (result) {
                case SQLITE_DONE:
                    return StepResult::DONE;
                case SQLITE_ROW:
                    return StepResult::ROW;
                case SQLITE_BUSY:
                    return StepResult::BUSY;
                default:
                    throw SQLite3StatementSQLError(result, msg_eval_failed, get_expanded_sql());
            }
        }

        int get_column_count() const { return sqlite3_column_count(stmt); }

        const char * get_column_database_name(int idx) const { return sqlite3_column_database_name(stmt, idx); }

        const char * get_column_table_name(int idx) const { return sqlite3_column_table_name(stmt, idx); }

        const char * get_column_origin_name(int idx) const { return sqlite3_column_origin_name(stmt, idx); }

        const char * get_column_name(int idx) const { return sqlite3_column_name(stmt, idx); }

        const char * get_sql() const { return sqlite3_sql(stmt); }

        const std::string get_expanded_sql() {
#if SQLITE_VERSION_NUMBER < 3014000
            // sqlite3_expanded_sql was added in sqlite 3.14; return sql instead
            return get_sql();
#else
            char * expanded_sql = sqlite3_expanded_sql(stmt);
            if (!expanded_sql) {
                throw SQLite3Error(msg_insufficient_memory);
            }
            std::string result(expanded_sql);
            sqlite3_free(expanded_sql);
            return result;
#endif
        }

        /// Reset prepared query to its initial state, ready to be re-executed.
        /// All the bound values remain untouched - retain their values.
        /// Use clearBindings if you need to reset them as well.
        void reset() { sqlite3_reset(stmt); }

        void clear_bindings() { sqlite3_clear_bindings(stmt); }

        template <typename T>
        T get(int idx) {
            return get(idx, Identity<T>{});
        }

        int64_t last_insert_rowid() { return db.last_insert_rowid(); }

        SQLite3 & get_db() const { return db; }

        ~Statement() { sqlite3_finalize(stmt); };

    protected:
        template <typename T>
        struct Identity {
            typedef T type;
        };

        /*template<typename TL>
        TL get(size_t, identity<TL>)
        {
            static_assert(sizeof(TL) == 0, "Not implemented");
        }*/

        int get(int idx, Identity<int> /*unused*/) { return sqlite3_column_int(stmt, idx); }

        uint32_t get(int idx, Identity<uint32_t> /*unused*/) {
            return static_cast<uint32_t>(sqlite3_column_int64(stmt, idx));
        }

        int64_t get(int idx, Identity<int64_t> /*unused*/) { return sqlite3_column_int64(stmt, idx); }

        double get(int idx, Identity<double> /*unused*/) { return sqlite3_column_double(stmt, idx); }

        bool get(int idx, Identity<bool> /*unused*/) { return sqlite3_column_int(stmt, idx) != 0; }

        const char * get(int idx, Identity<const char *> /*unused*/) {
            return reinterpret_cast<const char *>(sqlite3_column_text(stmt, idx));
        }

        std::string get(int idx, Identity<std::string> /*unused*/) {
            auto ret = reinterpret_cast<const char *>(sqlite3_column_text(stmt, idx));
            return ret ? ret : "";
        }

        Blob get(int idx, Identity<Blob> /*unused*/) {
            return {static_cast<size_t>(sqlite3_column_bytes(stmt, idx)), sqlite3_column_blob(stmt, idx)};
        }

        SQLite3 & db;
        sqlite3_stmt * stmt{};

    private:
        static const BgettextMessage msg_compilation_failed;
        static const BgettextMessage msg_bind_int_failed;
        static const BgettextMessage msg_bind_int64_failed;
        static const BgettextMessage msg_bind_uint32_failed;
        static const BgettextMessage msg_bind_double_failed;
        static const BgettextMessage msg_bind_bool_failed;
        static const BgettextMessage msg_bind_text_failed;
        static const BgettextMessage msg_bind_blob_failed;
        static const BgettextMessage msg_eval_failed;
        static const BgettextMessage msg_insufficient_memory;
    };

    class Query : public Statement {
    public:
        Query(SQLite3 & db, const char * sql) : Statement{db, sql} { map_cols_name(); }
        Query(SQLite3 & db, const std::string & sql) : Statement{db, sql} { map_cols_name(); }

        int get_column_index(const std::string & col_name) {
            auto it = cols_name_to_index.find(col_name);
            if (it == cols_name_to_index.end())
                throw SQLite3Error(msg_column_not_found, col_name);
            return it->second;
        }

        using Statement::get;

        template <typename T>
        T get(const std::string & col_name) {
            return get(get_column_index(col_name), Identity<T>{});
        }

    private:
        void map_cols_name() {
            for (int idx = 0; idx < get_column_count(); ++idx) {
                const char * name = get_column_name(idx);
                if (name)
                    cols_name_to_index[name] = idx;
            }
        }

        std::map<std::string, int> cols_name_to_index{};

        static const BgettextMessage msg_column_not_found;
    };

    SQLite3(const SQLite3 &) = delete;
    SQLite3 & operator=(const SQLite3 &) = delete;

    explicit SQLite3(const std::string & db_path) : path{db_path}, db{nullptr} { open(); }

    ~SQLite3() { close(); }

    const std::string & get_path() const { return path; }

    void open();
    void close();
    bool is_open() { return db != nullptr; };

    void exec(const char * sql) {
        auto result = sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
        if (result != SQLITE_OK) {
            throw SQLite3SQLError(result, msg_statement_exec_failed, std::string(sql));
        }
    }

    int changes() { return sqlite3_changes(db); }

    int64_t last_insert_rowid() { return sqlite3_last_insert_rowid(db); }

    std::string get_error() const { return sqlite3_errmsg(db); }

    void backup(const std::string & output_file);
    void restore(const std::string & input_file);

protected:
    std::string path;

    sqlite3 * db;

private:
    static const BgettextMessage msg_statement_exec_failed;
};


using SQLite3Ptr = std::shared_ptr<SQLite3>;

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_SQLITE3_SQLITE3_HPP
