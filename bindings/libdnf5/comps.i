#if defined(SWIGPYTHON)
%module(package="libdnf5") comps
#elif defined(SWIGPERL)
%module "libdnf5::comps"
#elif defined(SWIGRUBY)
%module "libdnf5/comps"
#endif

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

%include <shared.i>

%import "common.i"
%import "repo.i"
%import "transaction.i"

%{
    #include "libdnf/comps/group/package.hpp"
    #include "libdnf/comps/group/group.hpp"
    #include "libdnf/comps/group/query.hpp"
    #include "libdnf/comps/group/sack.hpp"
    #include "libdnf/comps/environment/environment.hpp"
    #include "libdnf/comps/environment/query.hpp"
    #include "libdnf/comps/environment/sack.hpp"
    #include "libdnf/comps/comps.hpp"
    #include "libdnf/repo/repo.hpp"
%}

#define CV __perl_CV

%template(SackQueryGroup) libdnf::sack::Query<libdnf::comps::Group>;
%template(SackQueryEnvironment) libdnf::sack::Query<libdnf::comps::Environment>;

%include "libdnf/comps/group/package.hpp"
%include "libdnf/comps/group/group.hpp"
%include "libdnf/comps/group/query.hpp"
%include "libdnf/comps/group/sack.hpp"
%include "libdnf/comps/environment/environment.hpp"
%include "libdnf/comps/environment/query.hpp"
%include "libdnf/comps/environment/sack.hpp"
%include "libdnf/comps/comps.hpp"

%template(CompsWeakPtr) libdnf::WeakPtr<libdnf::comps::Comps, false>;
