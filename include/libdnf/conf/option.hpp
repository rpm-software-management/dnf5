/*
Copyright (C) 2018-2021 Red Hat, Inc.

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

#ifndef LIBDNF_CONF_OPTION_HPP
#define LIBDNF_CONF_OPTION_HPP

#include "libdnf/common/exception.hpp"

#include <string>

namespace libdnf {

/// Option class is an abstract class. Parent of all options. Options are used to store a configuration.
/// @replaces libdnf:conf/Option.hpp:class:Option
class Option {
public:
    // TODO(jrohel): Prioroties are under discussion and probably will be modified.
    /// @replaces libdnf:conf/Option.hpp:enum class:Option::Priority
    enum class Priority {
        EMPTY = 0,
        DEFAULT = 10,
        MAINCONFIG = 20,
        AUTOMATICCONFIG = 30,
        REPOCONFIG = 40,
        PLUGINDEFAULT = 50,
        PLUGINCONFIG = 60,
        DROPINCONFIG = 65,
        COMMANDLINE = 70,
        RUNTIME = 80
    };

    /// Option exception
    class Exception : public RuntimeError {
    public:
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::Option"; }
        const char * get_name() const noexcept override { return "Exception"; }
        const char * get_description() const noexcept override { return "Option exception"; }
    };

    /// Exception that is generated when an invalid input value is detected.
    class InvalidValue : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "InvalidValue"; }
        const char * get_description() const noexcept override { return "Invalid value"; }
    };

    /// Exception that is thrown when a write to a locked option is detected.
    class WriteLocked : public LogicError {
    public:
        using LogicError::LogicError;
        const char * get_domain_name() const noexcept override { return "libdnf::Option"; }
        const char * get_name() const noexcept override { return "WriteLocked"; }
        const char * get_description() const noexcept override { return "Attempt to write to a locked option"; }
    };

    explicit Option(Priority priority = Priority::EMPTY);
    Option(const Option & src) = default;
    virtual ~Option() = default;

    /// Makes copy (clone) of this object.
    /// @replaces libdnf:conf/Option.hpp:method:Option.clone()
    virtual Option * clone() const = 0;

    /// Returns priority (source) of the stored value.
    /// @replaces libdnf:conf/Option.hpp:method:Option.getPriority()
    virtual Priority get_priority() const;

    /// Parses input string and sets new value and priority (source).
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    /// @replaces libdnf:conf/Option.hpp:method:Option.set(Priority priority, const std::string & value)
    virtual void set(Priority priority, const std::string & value) = 0;

    /// Gets a string representation of the stored value.
    /// @replaces libdnf:conf/Option.hpp:method:Option.getValueString()
    virtual std::string get_value_string() const = 0;

    /// Checks if the option is empty (has no stored value).
    /// @replaces libdnf:conf/Option.hpp:method:Option.empty()
    virtual bool empty() const noexcept;

    /// Locks the option.
    /// The locked option is read-only. Its value cannot be changed.
    ///
    /// @since 1.0
    void lock() noexcept;

    /// Checks if the option is locked.
    ///
    /// @return 'true' if the option is locked
    /// @since 1.0
    bool is_locked() const noexcept;

protected:
    void set_priority(Priority priority);

private:
    Priority priority;
    bool locked{false};
};

inline Option::Option(Priority priority) : priority(priority) {}

inline Option::Priority Option::get_priority() const {
    return priority;
}

inline bool Option::empty() const noexcept {
    return priority == Priority::EMPTY;
}

inline void Option::set_priority(Priority priority) {
    this->priority = priority;
}

inline void Option::lock() noexcept {
    locked = true;
}

inline bool Option::is_locked() const noexcept {
    return locked;
}

}  // namespace libdnf

#endif
