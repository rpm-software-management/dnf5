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
%typemap(out) libdnf5::comps::Group & libdnf5::SetConstIterator<libdnf5::comps::Group>::operator* = copy_return_value;
%template(SetConstIteratorGroup) libdnf5::SetConstIterator<libdnf5::comps::Group>;
%template(SetGroup) libdnf5::Set<libdnf5::comps::Group>;
%template(SackQueryGroup) libdnf5::sack::Query<libdnf5::comps::Group>;
%include "libdnf5/comps/group/query.hpp"
add_iterator(SetGroup)

%include "libdnf5/comps/environment/environment.hpp"
%typemap(out) libdnf5::comps::Environment & libdnf5::SetConstIterator<libdnf5::comps::Environment>::operator* = copy_return_value;
%template(SetConstIteratorEnvironment) libdnf5::SetConstIterator<libdnf5::comps::Environment>;
%template(SetEnvironment) libdnf5::Set<libdnf5::comps::Environment>;
%template(SackQueryEnvironment) libdnf5::sack::Query<libdnf5::comps::Environment>;
%include "libdnf5/comps/environment/query.hpp"
add_iterator(SetEnvironment)

%include "libdnf5/comps/comps_sack.hpp"
%template(CompsSackWeakPtr) libdnf5::WeakPtr<libdnf5::comps::CompsSack, false>;

// Deletes any previously defined catches
%catches();
