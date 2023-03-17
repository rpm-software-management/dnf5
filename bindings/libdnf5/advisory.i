#if defined(SWIGPYTHON)
%module(package="libdnf5") advisory
#elif defined(SWIGPERL)
%module "libdnf5::advisory"
#elif defined(SWIGRUBY)
%module "libdnf5/advisory"
#endif

%include <exception.i>
%include <std_vector.i>

%include <shared.i>

%import "common.i"

%exception {
    try {
        $action
    } catch (const libdnf::UserAssertionError & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const libdnf::Error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const std::runtime_error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

// TODO(jkolarik): advisory modules skipped for now

%{
    #include "libdnf/advisory/advisory.hpp"
    #include "libdnf/advisory/advisory_package.hpp"
    #include "libdnf/advisory/advisory_set.hpp"
    #include "libdnf/advisory/advisory_set_iterator.hpp"
    #include "libdnf/advisory/advisory_collection.hpp"
    #include "libdnf/advisory/advisory_query.hpp"
    #include "libdnf/advisory/advisory_reference.hpp"
%}

#define CV __perl_CV

%include "libdnf/advisory/advisory.hpp"
%include "libdnf/advisory/advisory_package.hpp"
%include "libdnf/advisory/advisory_set.hpp"

%rename(next) libdnf::advisory::AdvisorySetIterator::operator++();
%rename(value) libdnf::advisory::AdvisorySetIterator::operator*();
%include "libdnf/advisory/advisory_set_iterator.hpp"

%ignore libdnf::advisory::AdvisoryCollection::get_modules();
%include "libdnf/advisory/advisory_collection.hpp"
%include "libdnf/advisory/advisory_query.hpp"
%include "libdnf/advisory/advisory_reference.hpp"

%template(VectorAdvisoryCollection) std::vector<libdnf::advisory::AdvisoryCollection>;
%template(VectorAdvisoryPackage) std::vector<libdnf::advisory::AdvisoryPackage>;
%template(VectorAdvisoryReference) std::vector<libdnf::advisory::AdvisoryReference>;

add_iterator(AdvisorySet)
