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

#ifndef LIBDNF5_CONF_OPTION_HPP
#define LIBDNF5_CONF_OPTION_HPP

#include "option_errors.hpp"

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <string>


namespace libdnf5 {

/// Option class is an abstract class. Parent of all options. Options are used to store a configuration.
// @replaces libdnf:conf/Option.hpp:class:Option
class LIBDNF_API Option {
public:
    // TODO(jrohel): Prioroties are under discussion and probably will be modified.
    // @replaces libdnf:conf/Option.hpp:enum class:Option::Priority
    enum class Priority {
        EMPTY = 0,
        DEFAULT = 10,
        MAINCONFIG = 20,
        AUTOMATICCONFIG = 30,
        REPOCONFIG = 40,
        INSTALLROOT = 45,
        PLUGINDEFAULT = 50,
        PLUGINCONFIG = 60,
        COMMANDLINE = 70,
        RUNTIME = 80
    };

    explicit Option(Priority priority = Priority::EMPTY);
    Option(const Option & src);
    virtual ~Option();

    /// Makes copy (clone) of this object.
    // @replaces libdnf:conf/Option.hpp:method:Option.clone()
    virtual Option * clone() const = 0;

    /// Returns priority (source) of the stored value.
    // @replaces libdnf:conf/Option.hpp:method:Option.getPriority()
    virtual Priority get_priority() const;

    /// Parses input string and sets new value and priority (source).
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/Option.hpp:method:Option.set(Priority priority, const std::string & value)
    virtual void set(Priority priority, const std::string & value) = 0;

    /// Parses input string and sets new value and runtime priority.
    virtual void set(const std::string & value) = 0;

    /// Gets a string representation of the stored value.
    // @replaces libdnf:conf/Option.hpp:method:Option.getValueString()
    virtual std::string get_value_string() const = 0;

    /// Checks if the option is empty (has no stored value).
    // @replaces libdnf:conf/Option.hpp:method:Option.empty()
    virtual bool empty() const noexcept;

    /// Locks the option.
    /// The locked option is read-only. Its value cannot be changed.
    ///
    /// @param first_comment The comment will be saved when lock() is first called
    /// @since 1.0
    void lock(const std::string & first_comment);

    /// Checks if the option is locked.
    ///
    /// @return 'true' if the option is locked
    /// @since 1.0
    bool is_locked() const noexcept;

    /// Asserts the option is not locked and throws a `libdnf5::UserAssertionError` in case it is.
    ///
    /// @since 1.0
    void assert_not_locked() const;

protected:
    void set_priority(Priority priority);
    const std::string & get_lock_comment() const noexcept;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


}  // namespace libdnf5

#endif
