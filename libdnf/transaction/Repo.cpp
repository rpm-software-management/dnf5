/*
 * Copyright (C) 2017-2018 Red Hat, Inc.
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

#include "Repo.hpp"

namespace libdnf::transaction {

// initialize static variable Repo::cache
std::map< std::string, RepoPtr > Repo::cache;

Repo::Repo(libdnf::utils::SQLite3Ptr conn)
  : conn{conn}
{
}

void
Repo::save()
{
    dbSelectOrInsert();
}

void
Repo::dbInsert()
{
    const char *sql =
        "INSERT INTO "
        "  repo "
        "VALUES "
        "  (null, ?)";
    libdnf::utils::SQLite3::Statement query(*conn.get(), sql);
    query.bindv(getRepoId());
    query.step();
    setId(conn->last_insert_rowid());
}

std::shared_ptr< Repo >
Repo::getCached(libdnf::utils::SQLite3Ptr conn, const std::string &repoid)
{
    // HACK: this is kind of ugly - key is generated from repoid and sqlite3 pointer
    auto key = repoid + "/" + std::to_string(reinterpret_cast< std::size_t >(conn.get()));
    auto it = cache.find(key);
    if (it == cache.end()) {
        // cache miss
        auto repo = std::make_shared< Repo >(conn);
        repo->setRepoId(repoid);
        repo->save();
        cache[key] = repo;
        return repo;
    } else {
        // cache hit
        return it->second;
    }
}

void
Repo::dbSelectOrInsert()
{
    const char *sql =
        "SELECT "
        "  id "
        "FROM "
        "  repo "
        "WHERE "
        "  repoid = ? ";

    libdnf::utils::SQLite3::Statement query(*conn.get(), sql);
    query.bindv(getRepoId());
    libdnf::utils::SQLite3::Statement::StepResult result = query.step();

    if (result == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        setId(query.get< int >(0));
    } else {
        // insert and get the ID back
        dbInsert();
    }
}

}  // namespace libdnf::transaction
