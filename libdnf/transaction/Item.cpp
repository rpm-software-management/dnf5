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

#include "Item.hpp"

namespace libdnf {

Item::Item(libdnf::utils::SQLite3Ptr conn)
  : conn{conn}
{
}

void
Item::save()
{
    dbInsert();
}

void
Item::dbInsert()
{
    const char *sql =
        "INSERT INTO "
        "  item "
        "VALUES "
        "  (null, ?)";
    libdnf::utils::SQLite3::Statement query(*conn.get(), sql);
    query.bindv(static_cast< int >(itemType));
    query.step();
    setId(conn->last_insert_rowid());
}

std::string
Item::toStr() const
{
    return "<Item #" + std::to_string(getId()) + ">";
}

} // namespace libdnf
