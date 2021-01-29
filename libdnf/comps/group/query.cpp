#include "libdnf/comps/group/query.hpp"
#include "libdnf/comps/group/query_impl.hpp"
#include "libdnf/comps/group/sack.hpp"
#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/comps_impl.hpp"

extern "C" {
#include <solv/pool.h>
}

namespace libdnf::comps {


GroupQueryWeakPtr GroupQuery::get_weak_ptr() {
    return GroupQueryWeakPtr(this, &p_impl->data_guard);
}

GroupQuery::GroupQuery(GroupSack * sack)
    : Query()
    , sack(sack->get_weak_ptr())
    , p_impl{new Impl()}
{}


GroupQuery::GroupQuery(const GroupQuery & query)
    : Query(query)
    , sack(query.sack)
    , p_impl{new Impl()}
{}


GroupQuery::~GroupQuery() {}


}  // namespace libdnf::comps

