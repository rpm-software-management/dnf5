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

#ifndef LIBDNF_TRANSACTION_ITEM_HPP
#define LIBDNF_TRANSACTION_ITEM_HPP

#include <memory>
#include <string>

#include "libdnf/utils/sqlite3/sqlite3.hpp"

namespace libdnf {
class Item;
typedef std::shared_ptr< Item > ItemPtr;
}

#include "Types.hpp"

namespace libdnf {

class Item {
public:
    /// Default constructor.
    explicit Item(libdnf::utils::SQLite3Ptr conn);

    /// Default destructor.
    virtual ~Item() = default;

    /// Returns the ID of this item.
    int64_t getId() const noexcept { return id; }

    /// Sets the ID of this item.
    void setId(int64_t value) { id = value; }

    virtual ItemType getItemType() const noexcept { return itemType; }
    virtual std::string toStr() const;
    virtual void save();

protected:
    void dbInsert();

    libdnf::utils::SQLite3Ptr conn;
    int64_t id = 0;
    const ItemType itemType = ItemType::UNKNOWN;
};

} // namespace libdnf

#endif // LIBDNF_TRANSACTION_ITEM_HPP
