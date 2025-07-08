#if defined(SWIGPYTHON)
%module(package="libdnf5") comps
#elif defined(SWIGPERL)
%module "libdnf5::comps"
#elif defined(SWIGRUBY)
%module "libdnf5::comps"
#endif

%include <std_string.i>
%include <std_vector.i>

%include "shared.i"

%import "common.i"
%import "exception.i"
%import "repo.i"
%import "transaction.i"

%{
    #include "bindings/libdnf5/exception.hpp"

    #include "libdnf5/comps/group/package.hpp"
    #include "libdnf5/comps/group/group.hpp"
    #include "libdnf5/comps/group/query.hpp"
    #include "libdnf5/comps/environment/environment.hpp"
    #include "libdnf5/comps/environment/query.hpp"
    #include "libdnf5/comps/comps_sack.hpp"
    #include "libdnf5/repo/repo_query.hpp"
    #include "libdnf5/repo/repo.hpp"
%}

#define CV __perl_CV

// Deletes any previously defined general purpose exception handler
%exception;

// Set default exception handler
%catches(libdnf5::UserAssertionError, std::runtime_error, std::out_of_range);

%include "libdnf5/comps/group/package_type.hpp"
%include "libdnf5/comps/group/package.hpp"
%include "libdnf5/comps/group/group.hpp"
%template(VectorPackage) std::vector<libdnf5::comps::Package>;
%template(GroupWeakPtr) libdnf5::WeakPtr<libdnf5::comps::Group, false>;
%template(SetConstIteratorGroupWeakPtr) libdnf5::SetConstIterator<libdnf5::comps::GroupWeakPtr>;
%template(SetGroupWeakPtr) libdnf5::Set<libdnf5::comps::GroupWeakPtr>;
%template(SackQueryGroupWeakPtr) libdnf5::sack::Query<libdnf5::comps::GroupWeakPtr>;
%include "libdnf5/comps/group/query.hpp"
add_iterator(SetGroupWeakPtr)

%include "libdnf5/comps/environment/environment.hpp"
%template(EnvironmentWeakPtr) libdnf5::WeakPtr<libdnf5::comps::Environment, false>;
%template(SetConstIteratorEnvironmentWeakPtr) libdnf5::SetConstIterator<libdnf5::comps::EnvironmentWeakPtr>;
%template(SetEnvironmentWeakPtr) libdnf5::Set<libdnf5::comps::EnvironmentWeakPtr>;
%template(SackQueryEnvironmentWeakPtr) libdnf5::sack::Query<libdnf5::comps::EnvironmentWeakPtr>;
%include "libdnf5/comps/environment/query.hpp"
add_iterator(SetEnvironmentWeakPtr)

%include "libdnf5/comps/comps_sack.hpp"
%template(CompsSackWeakPtr) libdnf5::WeakPtr<libdnf5::comps::CompsSack, false>;

// Deletes any previously defined catches
%catches();
