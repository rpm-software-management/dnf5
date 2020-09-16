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


#ifndef LIBDNF_UTILS_SQLITE3_SQLITE3_HPP
#define LIBDNF_UTILS_SQLITE3_SQLITE3_HPP


#include "libdnf/utils/exception.hpp"

#include <sqlite3.h>

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


namespace libdnf::utils {


class SQLite3 {
public:
    class Error : public libdnf::Exception {
    public:
        explicit Error(const SQLite3 & s, int code, const std::string & msg)
            : libdnf::Exception("SQLite error on \"" + s.get_path() + "\": " + msg + ": " + s.get_error())
            , ecode{code} {}

        int code() const noexcept { return ecode; }
        const char * code_str() const noexcept { return sqlite3_errstr(ecode); }

    protected:
        int ecode;
    };

    struct Blob {
        size_t size;
        const void * data;
    };

    class Statement {
    public:
        /// An error class that will log the SQL statement in its constructor.
        class Error : public SQLite3::Error {
        public:
            Error(Statement & stmt, int code, const std::string & msg) : SQLite3::Error(stmt.db, code, msg) {
                // TODO(dmach): replace with a new logger
                // auto logger(libdnf::Log::getLogger());
                // logger->debug(std::string("SQL statement being executed: ") + stmt.getExpandedSql());
            }
        };

        enum class StepResult { DONE, ROW, BUSY };

        Statement(const Statement &) = delete;
        Statement & operator=(const Statement &) = delete;

        Statement(SQLite3 & db, const char * sql) : db(db) {
            auto result = sqlite3_prepare_v2(db.db, sql, -1, &stmt, nullptr);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Statement failed");
        };

        Statement(SQLite3 & db, const std::string & sql) : db(db) {
            auto result = sqlite3_prepare_v2(db.db, sql.c_str(), static_cast<int>(sql.length()) + 1, &stmt, nullptr);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Statement failed");
        };

        void bind(int pos, int val) {
            auto result = sqlite3_bind_int(stmt, pos, val);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Integer bind failed");
        }

        void bind(int pos, std::int64_t val) {
            auto result = sqlite3_bind_int64(stmt, pos, val);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Integer64 bind failed");
        }

        void bind(int pos, std::uint32_t val) {
            auto result = sqlite3_bind_int(stmt, pos, val);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Unsigned integer bind failed");
        }

        void bind(int pos, double val) {
            auto result = sqlite3_bind_double(stmt, pos, val);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Double bind failed");
        }

        void bind(int pos, bool val) {
            auto result = sqlite3_bind_int(stmt, pos, val ? 1 : 0);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Bool bind failed");
        }

        void bind(int pos, const char * val) {
            auto result = sqlite3_bind_text(stmt, pos, val, -1, SQLITE_TRANSIENT);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Text bind failed");
        }

        void bind(int pos, const std::string & val) {
            auto result = sqlite3_bind_text(stmt, pos, val.c_str(), -1, SQLITE_TRANSIENT);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Text bind failed");
        }

        void bind(int pos, const Blob & val) {
            auto result = sqlite3_bind_blob(stmt, pos, val.data, static_cast<int>(val.size), SQLITE_TRANSIENT);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Blob bind failed");
        }

        void bind(int pos, const std::vector<unsigned char> & val) {
            auto result = sqlite3_bind_blob(stmt, pos, val.data(), static_cast<int>(val.size()), SQLITE_TRANSIENT);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Blob bind failed");
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
                    throw Error(*this, result, "Reading a row failed");
            }
        }

        int get_column_count() const { return sqlite3_column_count(stmt); }

        const char * get_column_database_name(int idx) const { return sqlite3_column_database_name(stmt, idx); }

        const char * get_column_table_name(int idx) const { return sqlite3_column_table_name(stmt, idx); }

        const char * get_column_origin_name(int idx) const { return sqlite3_column_origin_name(stmt, idx); }

        const char * get_column_name(int idx) const { return sqlite3_column_name(stmt, idx); }

        const char * get_sql() const { return sqlite3_sql(stmt); }

        const char * get_expanded_sql() {
#if SQLITE_VERSION_NUMBER < 3014000
            // sqlite3_expanded_sql was added in sqlite 3.14; return sql instead
            return get_sql();
#else
            expanded_sql = sqlite3_expanded_sql(stmt);
            if (!expanded_sql) {
                throw libdnf::Exception(
                    "get_expanded_sql(): insufficient memory or result "
                    "exceed the maximum SQLite3 string length");
            }
            return expanded_sql;
#endif
        }

        void free_expanded_sql() { sqlite3_free(expanded_sql); }

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

        ~Statement() {
            free_expanded_sql();
            sqlite3_finalize(stmt);
        };

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

        uint32_t get(int idx, Identity<uint32_t> /*unused*/) { return sqlite3_column_int(stmt, idx); }

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
        char * expanded_sql{nullptr};
    };

    class Query : public Statement {
    public:
        Query(SQLite3 & db, const char * sql) : Statement{db, sql} { map_cols_name(); }
        Query(SQLite3 & db, const std::string & sql) : Statement{db, sql} { map_cols_name(); }

        int get_column_index(const std::string & col_name) {
            auto it = cols_name_to_index.find(col_name);
            if (it == cols_name_to_index.end())
                throw libdnf::Exception("get() column \"" + col_name + "\" not found");
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
            throw Error(*this, result, "Executing an SQL statement failed");
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
};


using SQLite3Ptr = std::shared_ptr<SQLite3>;


}  // namespace libdnf::utils


#endif  // LIBDNF_UTILS_SQLITE3_SQLITE3_HPP
