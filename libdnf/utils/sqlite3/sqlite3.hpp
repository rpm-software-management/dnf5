/* Sqlite3.hpp
 *
 * Copyright (C) 2017 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _SQLITE3_HPP
#define _SQLITE3_HPP

#include "../../error.hpp"
#include "../../log.hpp"

#include <sqlite3.h>

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class SQLite3 {
public:
    class Error : public libdnf::Error {
    public:
        Error(const SQLite3& s, int code, const std::string &msg) :
            libdnf::Error("SQLite error on \"" + s.getPath() + "\": " + msg + ": " + s.getError()),
            ecode{code}
        {}

        int code() const noexcept { return ecode; }
        const char *codeStr() const noexcept { return sqlite3_errstr(ecode); }

    protected:
        int ecode;
    };

    struct Blob {
        size_t size;
        const void *data;
    };

    class Statement {
    public:
        /**
         * An error class that will log the SQL statement in its constructor.
         */
        class Error : public SQLite3::Error {
        public:
            Error(Statement& stmt, int code, const std::string& msg) :
                SQLite3::Error(stmt.db, code, msg)
            {
                auto logger(libdnf::Log::getLogger());
                logger->debug(std::string("SQL statement being executed: ") + stmt.getExpandedSql());
            }
        };

        enum class StepResult { DONE, ROW, BUSY };

        Statement(const Statement &) = delete;
        Statement &operator=(const Statement &) = delete;

        Statement(SQLite3 &db, const char *sql)
          : db(db)
        {
            auto result = sqlite3_prepare_v2(db.db, sql, -1, &stmt, nullptr);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Statement failed");
        };

        Statement(SQLite3 &db, const std::string &sql)
          : db(db)
        {
            auto result = sqlite3_prepare_v2(db.db, sql.c_str(), sql.length() + 1, &stmt, nullptr);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Statement failed");
        };

        void bind(int pos, int val)
        {
            auto result = sqlite3_bind_int(stmt, pos, val);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Integer bind failed");
        }

        void bind(int pos, std::int64_t val)
        {
            auto result = sqlite3_bind_int64(stmt, pos, val);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Integer64 bind failed");
        }

        void bind(int pos, std::uint32_t val)
        {
            auto result = sqlite3_bind_int(stmt, pos, val);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Unsigned integer bind failed");
        }

        void bind(int pos, double val)
        {
            auto result = sqlite3_bind_double(stmt, pos, val);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Double bind failed");
        }

        void bind(int pos, bool val)
        {
            auto result = sqlite3_bind_int(stmt, pos, val ? 1 : 0);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Bool bind failed");
        }

        void bind(int pos, const char *val)
        {
            auto result = sqlite3_bind_text(stmt, pos, val, -1, SQLITE_TRANSIENT);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Text bind failed");
        }

        void bind(int pos, const std::string &val)
        {
            auto result = sqlite3_bind_text(stmt, pos, val.c_str(), -1, SQLITE_TRANSIENT);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Text bind failed");
        }

        void bind(int pos, const Blob &val)
        {
            auto result = sqlite3_bind_blob(stmt, pos, val.data, val.size, SQLITE_TRANSIENT);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Blob bind failed");
        }

        void bind(int pos, const std::vector< unsigned char > &val)
        {
            auto result = sqlite3_bind_blob(stmt, pos, val.data(), val.size(), SQLITE_TRANSIENT);
            if (result != SQLITE_OK)
                throw Error(*this, result, "Blob bind failed");
        }

        template< typename... Args >
        Statement &bindv(Args &&... args)
        {
            using Pass = int[];
            size_t pos{0};
            (void)Pass{(bind(++pos, args), 0)...};
            return *this;
        }

        StepResult step()
        {
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

        int getColumnCount() const { return sqlite3_column_count(stmt); }

        const char *getColumnDatabaseName(int idx) const
        {
            return sqlite3_column_database_name(stmt, idx);
        }

        const char *getColumnTableName(int idx) const
        {
            return sqlite3_column_table_name(stmt, idx);
        }

        const char *getColumnOriginName(int idx) const
        {
            return sqlite3_column_origin_name(stmt, idx);
        }

        const char *getColumnName(int idx) const { return sqlite3_column_name(stmt, idx); }

        const char *getSql() const { return sqlite3_sql(stmt); }

        const char *getExpandedSql()
        {
#if SQLITE_VERSION_NUMBER < 3014000
            // sqlite3_expanded_sql was added in sqlite 3.14; return sql instead
            return getSql();
#else
            expandSql = sqlite3_expanded_sql(stmt);
            if (!expandSql) {
                throw libdnf::Exception(
                    "getExpandedSql(): insufficient memory or result "
                    "exceed the maximum SQLite3 string length");
            }
            return expandSql;
#endif
        }

        void freeExpandedSql() { sqlite3_free(expandSql); }

        /**
         * Reset prepared query to its initial state, ready to be re-executed.
         * All the bound values remain untouched - retain their values.
         * Use clearBindings if you need to reset them as well.
         */
        void reset() { sqlite3_reset(stmt); }

        void clearBindings() { sqlite3_clear_bindings(stmt); }

        template< typename T >
        T get(int idx)
        {
            return get(idx, identity< T >{});
        }

        ~Statement()
        {
            freeExpandedSql();
            sqlite3_finalize(stmt);
        };

    protected:
        template< typename T >
        struct identity {
            typedef T type;
        };

        /*template<typename TL>
        TL get(size_t, identity<TL>)
        {
            static_assert(sizeof(TL) == 0, "Not implemented");
        }*/

        int get(int idx, identity< int >) { return sqlite3_column_int(stmt, idx); }

        uint32_t get(int idx, identity< uint32_t >) { return sqlite3_column_int(stmt, idx); }

        int64_t get(int idx, identity< int64_t >) { return sqlite3_column_int64(stmt, idx); }

        double get(int idx, identity< double >) { return sqlite3_column_double(stmt, idx); }

        bool get(int idx, identity< bool >) { return sqlite3_column_int(stmt, idx) != 0; }

        const char *get(int idx, identity< const char * >)
        {
            return reinterpret_cast< const char * >(sqlite3_column_text(stmt, idx));
        }

        std::string get(int idx, identity< std::string >)
        {
            auto ret = reinterpret_cast< const char * >(sqlite3_column_text(stmt, idx));
            return ret ? ret : "";
        }

        Blob get(int idx, identity< Blob >)
        {
            return {static_cast< size_t >(sqlite3_column_bytes(stmt, idx)),
                    sqlite3_column_blob(stmt, idx)};
        }

        SQLite3 &db;
        sqlite3_stmt *stmt;
        char *expandSql{nullptr};
    };

    class Query : public Statement {
    public:
        Query(SQLite3 &db, const char *sql)
          : Statement{db, sql}
        {
            mapColsName();
        }
        Query(SQLite3 &db, const std::string &sql)
          : Statement{db, sql}
        {
            mapColsName();
        }

        int getColumnIndex(const std::string &colName)
        {
            auto it = colsName2idx.find(colName);
            if (it == colsName2idx.end())
                throw libdnf::Exception("get() column \"" + colName + "\" not found");
            return it->second;
        }

        using Statement::get;

        template< typename T >
        T get(const std::string &colName)
        {
            return get(getColumnIndex(colName), identity< T >{});
        }

    private:
        void mapColsName()
        {
            for (int idx = 0; idx < getColumnCount(); ++idx) {
                const char *name = getColumnName(idx);
                if (name)
                    colsName2idx[name] = idx;
            }
        }

        std::map< std::string, int > colsName2idx;
    };

    SQLite3(const SQLite3 &) = delete;
    SQLite3 &operator=(const SQLite3 &) = delete;

    SQLite3(const std::string &dbPath)
      : path{dbPath}
      , db{nullptr}
    {
        open();
    }

    ~SQLite3() { close(); }

    const std::string &getPath() const { return path; }

    void open();
    void close();
    bool isOpened() { return db != nullptr; };

    void exec(const char *sql)
    {
        auto result = sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
        if (result != SQLITE_OK) {
            throw Error(*this, result, "Executing an SQL statement failed");
        }
    }

    int changes() { return sqlite3_changes(db); }

    int64_t lastInsertRowID() { return sqlite3_last_insert_rowid(db); }

    std::string getError() const { return sqlite3_errmsg(db); }

    void backup(const std::string &outputFile);
    void restore(const std::string &inputFile);

protected:
    std::string path;

    sqlite3 *db;
};

typedef std::shared_ptr< SQLite3 > SQLite3Ptr;

#endif
