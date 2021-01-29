#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/comps_impl.hpp"

extern "C" {
#include <solv/pool.h>
}


namespace libdnf::comps {

    
Comps::Impl::Impl() { pool = pool_create(); }


Comps::Impl::~Impl() { pool_free(pool); }


}  // namespace libdnf::comps
