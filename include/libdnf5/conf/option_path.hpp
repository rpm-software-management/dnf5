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

#ifndef LIBDNF5_CONF_OPTION_PATH_HPP
#define LIBDNF5_CONF_OPTION_PATH_HPP

#include "option_string.hpp"


namespace libdnf5 {

/// Exception that is generated when input path does not exist.
class OptionPathNotFoundError : public OptionValueNotAllowedError {
public:
    using OptionValueNotAllowedError::OptionValueNotAllowedError;
    const char * get_name() const noexcept override { return "OptionPathNotFoundError"; }
};


/// Option that stores file/directory path.
/// Support default value, and path verification (absolute, existence).
// @replaces libdnf:conf/OptionPath.hpp:class:OptionPath
class OptionPath : public OptionString {
public:
    /// Constructor sets default value and conditions.
    // @replaces libdnf:conf/OptionPath.hpp:ctor:OptionPath.OptionPath(const std::string & defaultValue, bool exists = false, bool absPath = false)
    explicit OptionPath(const std::string & default_value, bool exists = false, bool abs_path = false);

    /// Constructor sets default value and conditions.
    // @replaces libdnf:conf/OptionPath.hpp:ctor:OptionPath.OptionPath(const char * defaultValue, bool exists = false, bool absPath = false)
    explicit OptionPath(const char * default_value, bool exists = false, bool abs_path = false);

    /// Constructor sets default value and conditions.
    // @replaces libdnf:conf/OptionPath.hpp:ctor:OptionPath.OptionPath(const std::string & defaultValue, const std::string & regex, bool icase, bool exists = false, bool absPath = false)
    OptionPath(
        const std::string & default_value,
        const std::string & regex,
        bool icase,
        bool exists = false,
        bool abs_path = false);

    ~OptionPath();

    OptionPath(const OptionPath & src);

    /// Constructor sets default value and conditions.
    // @replaces libdnf:conf/OptionPath.hpp:ctor:OptionPath.OptionPath(const char * defaultValue, const std::string & regex, bool icase, bool exists = false, bool absPath = false)
    OptionPath(
        const char * default_value, const std::string & regex, bool icase, bool exists = false, bool abs_path = false);

    /// Makes copy (clone) of this object.
    // @replaces libdnf:conf/OptionPath.hpp:method:OptionPath.clone()
    OptionPath * clone() const override;

    /// Parses input string and sets new value and priority.
    /// According setting passed in constructor it can verify that the path is absolute, exists and match regex.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionPath.hpp:method:OptionPath.set(Priority priority, const std::string & value)
    void set(Priority priority, const std::string & value) override;

    /// Parses input string and sets new value and runtime priority.
    void set(const std::string & value) override;

    /// Tests input value and throws exception if the value is not allowed.
    // @replaces libdnf:conf/OptionPath.hpp:method:OptionPath.test(const std::string & value)
    void test(const std::string & value) const;

private:
    class Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
