#if defined(SWIGPYTHON)
%module(package="libdnf5") comps
#elif defined(SWIGPERL)
%module "libdnf5::comps"
#elif defined(SWIGRUBY)
%module "libdnf5::comps"
#endif

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

%include <shared.i>

%import "common.i"
%import "repo.i"
%import "transaction.i"

%exception {
    try {
        $action
    } catch (const std::exception &) {
        libdnf_exception_wrap_current()
        SWIG_fail;
    }
}

%{
    #include "libdnf5/comps/group/package.hpp"
    #include "libdnf5/comps/group/group.hpp"
    #include "libdnf5/comps/group/query.hpp"
    #include "libdnf5/comps/environment/environment.hpp"
    #include "libdnf5/comps/environment/query.hpp"
    #include "libdnf5/repo/repo_query.hpp"
    #include "libdnf5/repo/repo.hpp"

    // Exceptions
    #include "libdnf5/repo/file_downloader.hpp"
    #include "libdnf5/repo/package_downloader.hpp"
    #include "libdnf5/repo/repo_cache.hpp"
    #include "libdnf5/repo/repo_errors.hpp"
    #include "libdnf5/transaction/transaction.hpp"
    #include "libdnf5/transaction/transaction_item_action.hpp"
    #include "libdnf5/transaction/transaction_item_state.hpp"
%}

#define CV __perl_CV

%inline %{
    /// Fake function to force import of SWIG type "common.ExceptionWrap".
    libdnf5::common::ExceptionWrap _libdnf_comps_dummy() { return libdnf5::common::ExceptionWrap(); }
%}


%ignore libdnf5::comps::InvalidPackageType::InvalidPackageType;
%include "libdnf5/comps/group/package_type.hpp"

%include "libdnf5/comps/group/package.hpp"
%include "libdnf5/comps/group/group.hpp"
%template(VectorPackage) std::vector<libdnf5::comps::Package>;
%template(SetConstIteratorGroup) libdnf5::SetConstIterator<libdnf5::comps::Group>;
%template(SetGroup) libdnf5::Set<libdnf5::comps::Group>;
%template(SackQueryGroup) libdnf5::sack::Query<libdnf5::comps::Group>;
%include "libdnf5/comps/group/query.hpp"
add_iterator(SetGroup)

%include "libdnf5/comps/environment/environment.hpp"
%template(SetConstIteratorEnvironment) libdnf5::SetConstIterator<libdnf5::comps::Environment>;
%template(SetEnvironment) libdnf5::Set<libdnf5::comps::Environment>;
%template(SackQueryEnvironment) libdnf5::sack::Query<libdnf5::comps::Environment>;
%include "libdnf5/comps/environment/query.hpp"
add_iterator(SetEnvironment)

%exception;
