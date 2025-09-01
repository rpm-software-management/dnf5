// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef LIBDNF5_RPM_VERSIONLOCK_CONFIG_HPP
#define LIBDNF5_RPM_VERSIONLOCK_CONFIG_HPP

#include "libdnf5/common/sack/query_cmp.hpp"
#include "libdnf5/defs.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace libdnf5::rpm {

/// A condition for the versionlock package.
/// Each condition consist of three parts: key, comparison operator, and value.
/// Key can be one of "epoch", "evr", "arch".
/// Supported comparison operators are "<", "<=", "=", ">=", ">", "!=".
/// @since 5.1.13
class LIBDNF_API VersionlockCondition {
public:
    enum class Keys { EPOCH, EVR, ARCH };

    VersionlockCondition(const std::string & key_str, const std::string & comparator_str, const std::string & value);

    /// Returns true if this configuration entry is valid - contains supported values
    /// in all three parts (key, operator, and value).
    bool is_valid() const { return valid; }

    /// Get the key (which part of a package is compared).
    Keys get_key() const { return key; }
    /// Get the comparison operator.
    libdnf5::sack::QueryCmp get_comparator() const { return comparator; }
    /// Get the value.
    std::string get_value() const { return value; }

    /// Get the key as a string.
    std::string get_key_str() const { return key_str; }
    /// Get the comparison operator as a string.
    std::string get_comparator_str() const { return comparator_str; }

    /// Get list of errors found during parsing the entry from configuration file.
    const std::vector<std::string> & get_errors() const { return errors; }

    /// Converts the condition to "key operator value" string usable for printing.
    /// @param with_errors Include also error messages for invalid entries
    std::string to_string(bool with_errors) const;

private:
    // map string comparator to query cmp_type
    static const std::map<std::string, libdnf5::sack::QueryCmp> VALID_COMPARATORS;
    bool valid;
    std::string key_str;
    Keys key;
    std::string comparator_str;
    libdnf5::sack::QueryCmp comparator{0};
    std::string value;
    std::vector<std::string> errors{};
};

/// One versionlock configuration file entry. It consists of the
/// package name and a set of conditions. All conditions must be true
/// for package version to get locked.
/// @since 5.1.13
class LIBDNF_API VersionlockPackage {
public:
    /// Creates an instance of `VersionlockPackage` class specifying the
    /// name of package.
    /// @param name Name of the package to be configured
    VersionlockPackage(std::string_view name, std::vector<libdnf5::rpm::VersionlockCondition> && conditions);

    /// Returns true if this configuration entry is valid.
    bool is_valid() const { return valid; }

    /// Get the package name.
    std::string get_name() const { return name; }

    /// Get the comment for this entry.
    std::string get_comment() const { return comment; }
    /// Set comment for this entry.
    void set_comment(std::string_view comment);

    /// Get the list of conditions configured for the package.
    const std::vector<VersionlockCondition> & get_conditions() const { return conditions; }

    /// Add a new condition for the package
    void add_condition(VersionlockCondition && condition);

    /// Get list of errors found during parsing the entry from configuration file.
    const std::vector<std::string> & get_errors() const { return errors; }

    /// Converts the package configuration to string usable for printing.
    /// @param with_errors Include also error messages for invalid entries
    std::string to_string(bool with_errors, bool with_comment) const;

private:
    bool valid;
    std::string name;
    std::string comment;
    std::vector<VersionlockCondition> conditions{};
    std::vector<std::string> errors{};
};


/// Class contains parsed versionlock configuration file.
/// @since 5.1.13
class LIBDNF_API VersionlockConfig {
public:
    /// Get list of configured versionlock entries.
    std::vector<VersionlockPackage> & get_packages() { return packages; }

    /// Save configuration to the file specified in the constructor.
    void save();

private:
    friend class PackageSack;

    /// Creates an instance of `VersionlockConfig` specifying the config file
    /// to read.
    /// @param path Path to versionlock configuration file.
    LIBDNF_LOCAL VersionlockConfig(const std::filesystem::path & path);

    std::filesystem::path path;
    std::vector<VersionlockPackage> packages{};
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_VERSIONLOCK_CONFIG_HPP
