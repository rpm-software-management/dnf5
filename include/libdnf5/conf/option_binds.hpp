/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF5_CONF_OPTION_BINDS_HPP
#define LIBDNF5_CONF_OPTION_BINDS_HPP

#include "option.hpp"

#include <functional>
#include <map>


namespace libdnf5 {

struct OptionBindsError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "OptionBindsError"; }
};

class OptionBindsOptionNotFoundError : public OptionBindsError {
public:
    explicit OptionBindsOptionNotFoundError(const std::string & id);
    const char * get_name() const noexcept override { return "OptionBindsOptionNotFoundError"; }
};

class OptionBindsOptionAlreadyExistsError : public OptionBindsError {
public:
    explicit OptionBindsOptionAlreadyExistsError(const std::string & id);
    const char * get_name() const noexcept override { return "OptionBindsOptionAlreadyExistsError"; }
};

/// Maps the options names (text names read from config file, command line, ...) to options objects.
/// Supports user defined functions for processing new value and converting value to string.
class OptionBinds {
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

        Item(Option & option, NewStringFunc new_string_func, GetValueStringFunc get_value_string_func, bool add_value);
        explicit Item(Option & option);

        class Impl;
        ImplPtr<Impl> p_impl;
    };

    using Container = std::map<std::string, Item>;
    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;

    OptionBinds();
    OptionBinds(const OptionBinds & src);
    ~OptionBinds();

    Item & add(
        const std::string & id,
        Option & option,
        Item::NewStringFunc new_string_func,
        Item::GetValueStringFunc get_value_string_func,
        bool add_value);
    Item & add(const std::string & id, Option & option);
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
    class Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
