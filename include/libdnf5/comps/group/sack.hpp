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

#ifndef LIBDNF5_COMPS_GROUP_SACK_HPP
#define LIBDNF5_COMPS_GROUP_SACK_HPP

#include "libdnf5/common/sack/sack.hpp"
#include "libdnf5/common/weak_ptr.hpp"
#include "libdnf5/transaction/transaction_item_reason.hpp"

#include <map>
#include <string>


namespace libdnf5::comps {


class Comps;
class Group;

class GroupSack;
using GroupSackWeakPtr = WeakPtr<GroupSack, false>;


class GroupSack : public libdnf5::sack::Sack<Group> {
public:
    ~GroupSack();

    /// Create WeakPtr to GroupSack
    GroupSackWeakPtr get_weak_ptr();

    const std::map<std::string, libdnf5::transaction::TransactionItemReason> & get_reasons() const { return reasons; }

protected:
    explicit GroupSack(Comps & comps);

private:
    Comps & comps;

    WeakPtrGuard<GroupSack, false> sack_guard;

    /// @return Map of resolved reasons why groups were installed: ``{group_id -> reason}``.
    ///         A group can be installed due to multiple reasons, only the most significant is returned.
    /// @since 5.0
    std::map<std::string, libdnf5::transaction::TransactionItemReason> reasons;

    friend Comps;
    friend Group;

    friend class GroupQuery;
};


}  // namespace libdnf5::comps


#endif  // LIBDNF5_COMPS_GROUP_SACK_HPP
