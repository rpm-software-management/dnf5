#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/comps_impl.hpp>

extern "C" {
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/repo_comps.h>
}


namespace libdnf::comps {


Comps::Comps(libdnf::Base & base) : base{base}, p_impl{new Impl()} {}


Comps::~Comps() {}


}  // namespace libdnf::comps

