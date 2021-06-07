#include "libdnf/comps/group/sack.hpp"
#include "libdnf/comps/group/sack_impl.hpp"
#include "libdnf/comps/group/group.hpp"
#include "libdnf/comps/group/group-private.hpp"
#include "libdnf/comps/group/query.hpp"
#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/comps_impl.hpp"

extern "C" {
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solvable.h>
}

#include <map>


namespace libdnf::comps {


GroupSackWeakPtr GroupSack::get_weak_ptr() {
    return GroupSackWeakPtr(this, &p_impl->sack_guard);
}


GroupSack::GroupSack(Comps & comps)
    : Sack()
    , comps{comps}
    , p_impl{new Impl()}
{}


GroupSack::~GroupSack() {}


}  // namespace libdnf::comps
