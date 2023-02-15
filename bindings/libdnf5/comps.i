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

%include "libdnf/comps/group/package.hpp"
%include "libdnf/comps/group/group.hpp"
%template(SetConstIteratorGroup) libdnf::SetConstIterator<libdnf::comps::Group>;
%template(SetGroup) libdnf::Set<libdnf::comps::Group>;
%template(SackQueryGroup) libdnf::sack::Query<libdnf::comps::Group>;
%include "libdnf/comps/group/query.hpp"
%template(SackGroup) libdnf::sack::Sack<libdnf::comps::Group>;
%include "libdnf/comps/group/sack.hpp"
%template(GroupSackWeakPtr) libdnf::WeakPtr<libdnf::comps::GroupSack, false>;
add_iterator(SetGroup)

%include "libdnf/comps/environment/environment.hpp"
%template(SetConstIteratorEnvironment) libdnf::SetConstIterator<libdnf::comps::Environment>;
%template(SetEnvironment) libdnf::Set<libdnf::comps::Environment>;
%template(SackQueryEnvironment) libdnf::sack::Query<libdnf::comps::Environment>;
%include "libdnf/comps/environment/query.hpp"
%template(SackEnvironment) libdnf::sack::Sack<libdnf::comps::Environment>;
%include "libdnf/comps/environment/sack.hpp"
%template(EnvironmentSackWeakPtr) libdnf::WeakPtr<libdnf::comps::EnvironmentSack, false>;
add_iterator(SetEnvironment)

%include "libdnf/comps/comps.hpp"
%template(CompsWeakPtr) libdnf::WeakPtr<libdnf::comps::Comps, false>;
