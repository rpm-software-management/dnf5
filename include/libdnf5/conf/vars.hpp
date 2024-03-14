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

#ifndef LIBDNF5_CONF_VARS_HPP
#define LIBDNF5_CONF_VARS_HPP

#include "libdnf5/base/base_weak.hpp"

#include <map>
#include <string>
#include <vector>

namespace libdnf5 {

// Thrown when attempting to set a read-only variable
class ReadOnlyVariableError : public Error {
    using Error::Error;

    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "ReadOnlyVariableError"; }
};

/// @class Vars
///
/// @brief Class for reading and substituting DNF vars (arch, releasever, etc.).
///
/// The class loads the variables from the environment as well as from a list
/// of directories.
struct LIBDNF_API Vars {
public:
    enum class Priority {
        DEFAULT = 10,
        AUTO = 20,
        VARSDIR = 30,
        PLUGIN = 40,
        ENVIRONMENT = 50,
        COMMANDLINE = 60,
        RUNTIME = 70
    };

    struct Variable {
        std::string value;
        Priority priority;
    };

    Vars(const libdnf5::BaseWeakPtr & base);
    Vars(libdnf5::Base & base);

    ~Vars();

    /// @brief Substitute DNF vars in the input text.
    ///
    /// @param text The text for substitution
    /// @return The substituted text
    std::string substitute(const std::string & text) const;

    const std::map<std::string, Variable> & get_variables() const;

    /// @brief Set particular variable to a value
    ///
    /// @param name Name of the variable
    /// @param value Value to be stored in variable
    /// @param prio Source/Priority of the value
    /// @throw ReadOnlyVariableError if the variable is read-only
    void set(const std::string & name, const std::string & value, Priority prio = Priority::RUNTIME);

    /// @brief Unset particular variable
    ///
    /// @param name Name of the variable
    /// @param prio Source/Priority of the request
    /// @throw ReadOnlyVariableError if the variable is read-only
    /// @return false if the variable exists after the function returns (insufficient request priority)
    bool unset(const std::string & name, Priority prio = Priority::RUNTIME);

    /// @brief Checks whether a variable is read-only
    ///
    /// @param name Name of the variable
    /// @return true if the variable is read-only, false if it is writable
    bool is_read_only(const std::string & name) const;

    /// @brief Checks if there is an variable with name equivalent to name in the container.
    ///
    /// @param name Name of the variable
    /// @return true if there is such an element, otherwise false
    bool contains(const std::string & name) const;

    /// @brief Get value of particular variable.
    ///
    /// @param name Name of the variable
    const std::string & get_value(const std::string & name) const;

    /// @brief Get particular variable.
    ///
    /// @param name Name of the variable
    const Variable & get(const std::string & name) const;

    static std::unique_ptr<std::string> detect_release(const BaseWeakPtr & base, const std::string & install_root_path);

private:
    friend class Base;

    /// @brief Loads DNF vars from the environment and the passed directories.
    ///
    /// Environment variables are loaded first, then the directories are loaded
    /// in the order they are stored in the vector. Variables will overwrite
    /// any previous occurrence of a variable with the same name.
    ///
    /// @param installroot The path to the installroot
    /// @param directories The directories to load vars from
    LIBDNF_LOCAL void load(const std::string & installroot, const std::vector<std::string> & directories);

    /// @brief Detects the system's arch, basearch and relesever.
    ///
    /// @param installroot The installroot directory
    LIBDNF_LOCAL void detect_vars(const std::string & installroot);

    /// @brief Loads DNF vars from a directory.
    ///
    /// Each file in the directory represents one variable. The filename stands
    /// for the name of the variable and the value is the first line of the
    /// file's contents.
    ///
    /// @param directory Path to a directory with DNF vars
    LIBDNF_LOCAL void load_from_dir(const std::string & directory);

    /// @brief Loads DNF vars from the environment.
    ///
    /// Reads environment variables that match "DNF[0-9]" and
    /// "DNF_VAR_[A-Za-z0-9_]+" patterns. The "DNF_VAR_" prefix is cut off.
    LIBDNF_LOCAL void load_from_env();

    /// @brief Set a variable to a value, only obtaining the value if needed using `get_value`
    ///
    /// @param name Name of the variable
    /// @param get_value Function that returns the (optional) value for the variable
    /// @param prio Source/Priority of the value
    /// @throw ReadOnlyVariableError if the variable is read-only
    LIBDNF_LOCAL void set_lazy(
        const std::string & name,
        const std::function<const std::unique_ptr<const std::string>()> & get_value,
        Priority prio);

    /// @brief Expand variables in a subexpression
    ///
    /// @param text String with variable expressions
    /// @param depth The recursive depth
    /// @return Pair of the resulting string and the number of characters scanned in `text`
    LIBDNF_LOCAL std::pair<std::string, size_t> substitute_expression(std::string_view text, unsigned int depth) const;

    /// @brief Split releasever on the first "." into its "major" and "minor" components
    ///
    /// @param releasever A releasever string, possibly containing a "."
    /// @return releasever_major, releasever_minor
    LIBDNF_LOCAL static std::tuple<std::string, std::string> split_releasever(const std::string & releasever);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
