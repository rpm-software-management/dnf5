// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CONF_OPTION_BINDS_HPP
#define LIBDNF5_CONF_OPTION_BINDS_HPP

#include "option.hpp"
#include "option_binds_errors.hpp"

#include "libdnf5/defs.h"

#include <functional>
#include <map>


namespace libdnf5 {

/// Maps the options names (text names read from config file, command line, ...) to options objects.
/// Supports user defined functions for processing new value and converting value to string.
class LIBDNF_API OptionBinds {
public:
    /// Extends the option with user-defined functions for processing a new value and converting value to a string.
    /// It is used as additional level of processing when the option is accessed by its text name.
    class Item final {
    public:
        using NewStringFunc = std::function<void(Option::Priority, const std::string &)>;
        using GetValueStringFunc = std::function<const std::string &()>;

        ~Item();
        Option::Priority get_priority() const;
        void new_string(Option::Priority priority, const std::string & value);
        std::string get_value_string() const;
        bool get_is_append_option() const;

    private:
        friend class OptionBinds;

        LIBDNF_LOCAL Item(
            Option & option, NewStringFunc new_string_func, GetValueStringFunc get_value_string_func, bool add_value);
        LIBDNF_LOCAL explicit Item(Option & option);

        class LIBDNF_LOCAL Impl;
        ImplPtr<Impl> p_impl;
    };

    using Container = std::map<std::string, Item>;
    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;

    OptionBinds();
    OptionBinds(const OptionBinds & src);
    ~OptionBinds();

    Item & add(
        std::string id,
        Option & option,
        Item::NewStringFunc new_string_func,
        Item::GetValueStringFunc get_value_string_func,
        bool add_value);
    Item & add(std::string id, Option & option);
    Item & at(const std::string & id);
    const Item & at(const std::string & id) const;
    bool empty() const noexcept;
    std::size_t size() const noexcept;
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;
    iterator find(const std::string & id);
    const_iterator find(const std::string & id) const;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
