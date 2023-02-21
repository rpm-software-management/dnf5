#if defined(SWIGPYTHON)
%module(package="libdnf5") rpm
#elif defined(SWIGPERL)
%module "libdnf5::rpm"
#elif defined(SWIGRUBY)
%module "libdnf5/rpm"
#endif

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

%include <shared.i>

%import "common.i"
%import "conf.i"
%import "repo.i"
%import "transaction.i"

%exception {
    try {
        $action
    } catch (const std::runtime_error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const libdnf::UserAssertionError & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

%{
    #include "libdnf/advisory/advisory_module.hpp"
    #include "libdnf/advisory/advisory_query.hpp"
    #include "libdnf/rpm/checksum.hpp"
    #include "libdnf/rpm/nevra.hpp"
    #include "libdnf/rpm/package.hpp"
    #include "libdnf/rpm/package_query.hpp"
    #include "libdnf/rpm/package_sack.hpp"
    #include "libdnf/rpm/package_set.hpp"
    #include "libdnf/rpm/package_set_iterator.hpp"
    #include "libdnf/rpm/reldep.hpp"
    #include "libdnf/rpm/reldep_list.hpp"
    #include "libdnf/rpm/reldep_list_iterator.hpp"
    #include "libdnf/rpm/transaction_callbacks.hpp"
%}

#define CV __perl_CV

%include "libdnf/rpm/checksum.hpp"

%ignore NevraIncorrectInputError;
%include "libdnf/rpm/nevra.hpp"

%template(VectorNevra) std::vector<libdnf::rpm::Nevra>;
%template(VectorNevraForm) std::vector<libdnf::rpm::Nevra::Form>;
%template(PairBoolNevra) std::pair<bool, libdnf::rpm::Nevra>;

%include "libdnf/rpm/package_sack.hpp"
%template(PackageSackWeakPtr) libdnf::WeakPtr<libdnf::rpm::PackageSack, false>;

%include "libdnf/rpm/reldep.hpp"

%rename(next) libdnf::rpm::ReldepListIterator::operator++();
%rename(value) libdnf::rpm::ReldepListIterator::operator*();
%include "libdnf/rpm/reldep_list_iterator.hpp"
%include "libdnf/rpm/reldep_list.hpp"
%include "libdnf/rpm/package.hpp"

%rename(next) libdnf::rpm::PackageSetIterator::operator++();
%rename(value) libdnf::rpm::PackageSetIterator::operator*();
%include "libdnf/rpm/package_set_iterator.hpp"
%include "libdnf/rpm/package_set.hpp"

%ignore libdnf::rpm::PackageQuery::PackageQuery(PackageQuery && src);
%include "libdnf/rpm/package_query.hpp"

add_iterator(PackageSet)
add_iterator(ReldepList)

%feature("director") TransactionCallbacks;
%include "libdnf/rpm/transaction_callbacks.hpp"
wrap_unique_ptr(TransactionCallbacksUniquePtr, libdnf::rpm::TransactionCallbacks);
