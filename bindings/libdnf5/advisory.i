#if defined(SWIGPYTHON)
%module(package="libdnf5") advisory
#elif defined(SWIGPERL)
%module "libdnf5::advisory"
#elif defined(SWIGRUBY)
%module "libdnf5::advisory"
#endif

#if defined(SWIGRUBY)
%mixin libdnf5::advisory::AdvisorySet "Enumerable";
#endif

%include <exception.i>
%include <std_vector.i>

%include <shared.i>

%import "common.i"

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
    #include "libdnf5/advisory/advisory.hpp"
    #include "libdnf5/advisory/advisory_package.hpp"
    #include "libdnf5/advisory/advisory_module.hpp"
    #include "libdnf5/advisory/advisory_set.hpp"
    #include "libdnf5/advisory/advisory_set_iterator.hpp"
    #include "libdnf5/advisory/advisory_collection.hpp"
    #include "libdnf5/advisory/advisory_query.hpp"
    #include "libdnf5/advisory/advisory_reference.hpp"
%}

#define CV __perl_CV

%include "libdnf5/advisory/advisory.hpp"
%include "libdnf5/advisory/advisory_package.hpp"
%include "libdnf5/advisory/advisory_set.hpp"

%rename(next) libdnf5::advisory::AdvisorySetIterator::operator++();
%rename(value) libdnf5::advisory::AdvisorySetIterator::operator*();
%include "libdnf5/advisory/advisory_set_iterator.hpp"

%include "libdnf5/advisory/advisory_module.hpp"
%include "libdnf5/advisory/advisory_collection.hpp"
%include "libdnf5/advisory/advisory_query.hpp"
%include "libdnf5/advisory/advisory_reference.hpp"

%template(VectorAdvisoryModule) std::vector<libdnf5::advisory::AdvisoryModule>;
%template(VectorAdvisoryCollection) std::vector<libdnf5::advisory::AdvisoryCollection>;
%template(VectorAdvisoryPackage) std::vector<libdnf5::advisory::AdvisoryPackage>;
%template(VectorAdvisoryReference) std::vector<libdnf5::advisory::AdvisoryReference>;

add_iterator(AdvisorySet)

#if defined(SWIGRUBY)
fix_swigtype_trait(libdnf5::advisory::Advisory)
#endif
add_ruby_each(libdnf5::advisory::AdvisorySet)
