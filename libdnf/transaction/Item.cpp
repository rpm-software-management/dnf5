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
#include "libdnf/transaction/db/item.hpp"

namespace libdnf::transaction {

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
    auto query = item_insert_new_query(conn, itemType);
    setId(item_insert(*query));
}

std::string
Item::toStr() const
{
    return "<Item #" + std::to_string(getId()) + ">";
}

}  // namespace libdnf::transaction
