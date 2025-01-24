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
    } catch (const libdnf5::UserAssertionError & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const libdnf5::Error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const std::runtime_error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
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
%}

#define CV __perl_CV

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
