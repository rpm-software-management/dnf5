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

#ifndef LIBDNF5_COMPS_ENVIRONMENT_ENVIRONMENT_HPP
#define LIBDNF5_COMPS_ENVIRONMENT_ENVIRONMENT_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/transaction/transaction_item_reason.hpp"

#include <set>
#include <string>
#include <vector>


namespace libdnf5::comps {


struct EnvironmentId {
public:
    EnvironmentId() = default;
    explicit EnvironmentId(int id) : id(id) {}

    bool operator==(const EnvironmentId & other) const noexcept { return id == other.id; }
    bool operator!=(const EnvironmentId & other) const noexcept { return id != other.id; }

    // Corresponds to solvable id
    int id{0};
};


// @replaces dnf:dnf/comps.py:class:Environment
class Environment {
public:
    ~Environment();

    Environment(const Environment & src);
    Environment & operator=(const Environment & src);

    Environment(Environment && src) noexcept;
    Environment & operator=(Environment && src) noexcept;

    /// @return The Environment id.
    /// @since 5.0
    std::string get_environmentid() const;

    /// @return The Environment name.
    /// @since 5.0
    std::string get_name() const;

    /// @return The Environment description.
    /// @since 5.0
    std::string get_description() const;

    /// @return The translated name of the Environment based on current locales.
    ///         If no translation is found, return untranslated name.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Environment.ui_name
    std::string get_translated_name(const char * lang) const;
    std::string get_translated_name() const;

    /// @return The translated description of the Environment based on current locales.
    ///         If no translation is found, return untranslated description.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Environment.ui_description
    std::string get_translated_description(const char * lang) const;
    std::string get_translated_description() const;

    /// @return The Environment display order.
    /// @since 5.0
    //
    // TODO(pkratoch): respect the display_order when listing environments
    std::string get_order() const;

    /// @return std::vector of Group ids belonging to the Environment.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Environment.mandatory_groups
    std::vector<std::string> get_groups();

    /// @return std::vector of optional Group ids belonging to the Environment.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Environment.optional_groups
    std::vector<std::string> get_optional_groups();

    /// @return std::set of names of repositories that contain the Environment.
    /// @since 5.0
    //
    // TODO(pkratoch): Either remove this method, or return a vector of the weak pointers to the repo objects
    std::set<std::string> get_repos() const;

    /// @return `true` if the Environment is installed (belongs to the \@System repo).
    /// @since 5.0
    bool get_installed() const;

    /// @return Resolved reason why the Environment was installed.
    ///         Environments can be installed due to multiple reasons, only the most significant is returned.
    /// @since 5.0
    libdnf5::transaction::TransactionItemReason get_reason() const;

    /// Merge the Environment with another one.
    /// @since 5.0
    Environment & operator+=(const Environment & rhs);

    // Environments are the same if they have the same environment_ids (libsolv solvable_ids) in the same order, and the same base.
    bool operator==(const Environment & rhs) const noexcept;

    bool operator!=(const Environment & rhs) const noexcept;

    // Compare Environments by environmentid and then by repoids (the string ids, so that it's deterministic even when loaded in a different order).
    bool operator<(const Environment & rhs) const;

    /// Serialize the Environment into an xml file
    /// @param path Path of the output xml file.
    /// @exception utils::xml::XMLSaveError When saving of the file fails.
    /// @since 5.0
    void serialize(const std::string & path);

protected:
    explicit Environment(const libdnf5::BaseWeakPtr & base);
    explicit Environment(libdnf5::Base & base);

private:
    friend class EnvironmentQuery;

    void add_environment_id(const EnvironmentId & environment_id);

    class Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf5::comps


#endif  // LIBDNF5_COMPS_ENVIRONMENT_ENVIRONMENT_HPP
