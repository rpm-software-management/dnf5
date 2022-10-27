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

#ifndef LIBDNF_COMPS_GROUP_GROUP_HPP
#define LIBDNF_COMPS_GROUP_GROUP_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/comps/group/package.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <set>
#include <string>
#include <vector>


namespace libdnf::comps {


struct GroupId {
public:
    GroupId() = default;
    explicit GroupId(int id) : id(id) {}

    bool operator==(const GroupId & other) const noexcept { return id == other.id; }
    bool operator!=(const GroupId & other) const noexcept { return id != other.id; }

    // Corresponds to solvable id
    int id{0};
};


// @replaces dnf:dnf/comps.py:class:Group
class Group {
public:
    /// @return The Group id.
    /// @since 5.0
    std::string get_groupid() const;

    /// @return The Group name.
    /// @since 5.0
    std::string get_name() const;

    /// @return The Group description.
    /// @since 5.0
    std::string get_description() const;

    /// @return The translated name of the Group based on current locales.
    ///         If no translation is found, return untranslated name.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Group.ui_name
    std::string get_translated_name(const char * lang) const;
    std::string get_translated_name() const;

    /// @return The translated description of the Group based on current locales.
    ///         If no translation is found, return untranslated ui_description.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Group.ui_description
    std::string get_translated_description(const char * lang) const;
    std::string get_translated_description() const;

    /// @return The Group display order.
    /// @since 5.0
    //
    // TODO(pkratoch): respect the display_order when listing groups
    std::string get_order() const;

    /// @return The Group langonly.
    /// @since 5.0
    std::string get_langonly() const;

    /// @return `true` if the Group is visible to the users.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Group.visible
    bool get_uservisible() const;

    /// @return `true` if the Group is installed by default.
    /// @since 5.0
    bool get_default() const;

    /// @return std::vector of Packages belonging to the Group.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:method:Group.packages_iter(self)
    std::vector<Package> get_packages();

    /// @return std::vector of Packages of given type belonging to the Group.
    /// @param type One of the PackageTypes.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Group.conditional_packages
    // @replaces dnf:dnf/comps.py:attribute:Group.default_packages
    // @replaces dnf:dnf/comps.py:attribute:Group.mandatory_packages
    // @replaces dnf:dnf/comps.py:attribute:Group.optional_packages
    std::vector<Package> get_packages_of_type(PackageType type);

    /// @return std::set of names of repositories that contain the Group.
    /// @since 5.0
    //
    // TODO(pkratoch): Either remove this method, or return a vector of the weak pointers to the repo objects
    std::set<std::string> get_repos() const;

    /// @return `true` if the Group is installed (belongs to the @System repo).
    /// @since 5.0
    bool get_installed() const;

    /// @return Resolved reason why the Group was installed.
    ///         Groups can be installed due to multiple reasons, only the most significant is returned.
    /// @since 5.0
    //
    // TODO(dmach): return actual value from data in GroupSack
    // TODO(dmach): throw an exception when getting a reason for an available package (it should work only for installed)
    libdnf::transaction::TransactionItemReason get_reason() const {
        return libdnf::transaction::TransactionItemReason::NONE;
    }

    /// Merge the Group with another one.
    /// @since 5.0
    Group & operator+=(const Group & rhs);

    // Groups are the same if they have the same group_ids (libsolv solvable_ids) in the same order, and the same base.
    bool operator==(const Group & rhs) const noexcept { return group_ids == rhs.group_ids && base == rhs.base; }
    bool operator!=(const Group & rhs) const noexcept { return group_ids != rhs.group_ids || base != rhs.base; }
    // Compare Groups by groupid and then by repoids (the string ids, so that it's deterministic even when loaded in a different order).
    bool operator<(const Group & rhs) const {
        return get_groupid() < rhs.get_groupid() || get_repos() < rhs.get_repos();
    }

    /// Serialize the Group into an xml file.
    /// @param path Path of the output xml file.
    /// @exception utils::xml::XMLSaveError When saving of the file fails.
    /// @since 5.0
    void serialize(const std::string & path);

protected:
    explicit Group(const BaseWeakPtr & base);
    explicit Group(libdnf::Base & base);

private:
    libdnf::BaseWeakPtr base;

    // Corresponds to solvable ids for this group (libsolv doesn't merge groups, so there are multiple solvables
    // for one groupid).
    // The order is given by the repoids of the originating repositories. The repoids that come later in the alphabet
    // are preferred (e.g. the description is taken from solvable from repository "B", even though there is a different
    // description in repository "A"). A notable case are repositories "fedora" and "updates" where the "updates"
    // repository is preferred, and coincidentally, this is what we want, because "updates" contains more up-to-date
    // definitions.
    std::vector<GroupId> group_ids;

    std::vector<Package> packages;

    friend class GroupSack;
    friend class GroupQuery;
};


}  // namespace libdnf::comps


#endif  // LIBDNF_COMPS_GROUP_GROUP_HPP
