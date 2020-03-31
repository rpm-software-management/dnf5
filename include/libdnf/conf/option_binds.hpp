/*
 * Copyright (C) 2018 Red Hat, Inc.
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

#ifndef _LIBDNF_OPTION_BINDS_HPP
#define _LIBDNF_OPTION_BINDS_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "Option.hpp"

#include <functional>
#include <map>

namespace libdnf {

class OptionBinds {
public:
    struct Exception : public std::runtime_error {
        Exception(const std::string & what) : runtime_error(what) {}
    protected:
        mutable std::string tmpMsg;
    };
    struct OutOfRange : public Exception {
        OutOfRange(const std::string & id) : Exception(id) {}
        const char * what() const noexcept override;
    };
    struct AlreadyExists : public Exception {
        AlreadyExists(const std::string & id) : Exception(id) {}
        const char * what() const noexcept override;
    };

    class Item final {
    public:
        typedef std::function<void(Option::Priority, const std::string &)> NewStringFunc;
        typedef std::function<const std::string & ()> GetValueStringFunc;

        Option::Priority getPriority() const;
        void newString(Option::Priority priority, const std::string & value);
        std::string getValueString() const;
        bool getAddValue() const;

    private:
        friend class OptionBinds;

        Item(Option & option, const NewStringFunc & newString,
             const GetValueStringFunc & getValueString, bool addValue);
        Item(Option & option, NewStringFunc && newString,
             GetValueStringFunc && getValueString, bool addValue);
        Item(Option & option);
        Option * option;
        NewStringFunc newStr;
        GetValueStringFunc getValueStr;
        bool addValue{false}; // hint that new value be added
    };

    typedef std::map<std::string, Item> Container;
    typedef Container::iterator iterator;
    typedef Container::const_iterator const_iterator;

    Item & add(const std::string & id, Option & option, const Item::NewStringFunc & newString,
                 const Item::GetValueStringFunc & getValueString, bool addValue);
    Item & add(const std::string & id, Option & option, Item::NewStringFunc && newString,
                 Item::GetValueStringFunc && getValueString, bool addValue);
    Item & add(const std::string & id, Option & option);
    Item & at(const std::string & id);
    const Item & at(const std::string & id) const;
    bool empty() const noexcept { return items.empty(); }
    std::size_t size() const noexcept { return items.size(); }
    iterator begin() noexcept { return items.begin(); }
    const_iterator begin() const noexcept { return items.begin(); }
    const_iterator cbegin() const noexcept { return items.cbegin(); }
    iterator end() noexcept { return items.end(); }
    const_iterator end() const noexcept { return items.end(); }
    const_iterator cend() const noexcept { return items.cend(); }
    iterator find(const std::string & id) { return items.find(id); }
    const_iterator find(const std::string & id) const { return items.find(id); }

private:
    Container items;
};

}

#endif

#endif
