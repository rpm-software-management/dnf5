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
    } catch (const libdnf5::UserAssertionError & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const libdnf5::Error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const std::runtime_error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}


%{
    #include "libdnf5/rpm/arch.hpp"
    #include "libdnf5/rpm/checksum.hpp"
    #include "libdnf5/rpm/nevra.hpp"
    #include "libdnf5/rpm/package.hpp"
    #include "libdnf5/rpm/package_query.hpp"
    #include "libdnf5/rpm/package_sack.hpp"
    #include "libdnf5/rpm/package_set.hpp"
    #include "libdnf5/rpm/package_set_iterator.hpp"
    #include "libdnf5/rpm/reldep.hpp"
    #include "libdnf5/rpm/reldep_list.hpp"
    #include "libdnf5/rpm/reldep_list_iterator.hpp"
    #include "libdnf5/rpm/rpm_signature.hpp"
    #include "libdnf5/rpm/transaction_callbacks.hpp"
    #include "libdnf5/rpm/versionlock_config.hpp"
%}

#define CV __perl_CV

%include "libdnf5/rpm/arch.hpp"

%include "libdnf5/rpm/checksum.hpp"

%ignore NevraIncorrectInputError;
%include "libdnf5/rpm/nevra.hpp"

%template(VectorNevra) std::vector<libdnf5::rpm::Nevra>;
%template(VectorNevraForm) std::vector<libdnf5::rpm::Nevra::Form>;
%template(PairBoolNevra) std::pair<bool, libdnf5::rpm::Nevra>;

%include "libdnf5/rpm/versionlock_config.hpp"

%template(VectorVersionlockCondition) std::vector<libdnf5::rpm::VersionlockCondition>;
%template(VectorVersionlockPackage) std::vector<libdnf5::rpm::VersionlockPackage>;

%include "libdnf5/rpm/package_sack.hpp"
%template(PackageSackWeakPtr) libdnf5::WeakPtr<libdnf5::rpm::PackageSack, false>;

add_str(libdnf5::rpm::Reldep)
add_repr(libdnf5::rpm::Reldep)
add_hash(libdnf5::rpm::Reldep)
%include "libdnf5/rpm/reldep.hpp"

%rename(next) libdnf5::rpm::ReldepListIterator::operator++();
%rename(value) libdnf5::rpm::ReldepListIterator::operator*();
%include "libdnf5/rpm/reldep_list_iterator.hpp"
%include "libdnf5/rpm/reldep_list.hpp"

add_str(libdnf5::rpm::Package)
add_repr(libdnf5::rpm::Package)
add_hash(libdnf5::rpm::Package)
%include "libdnf5/rpm/package.hpp"

%template(VectorPackage) std::vector<libdnf5::rpm::Package>;
%template(VectorVectorPackage) std::vector<std::vector<libdnf5::rpm::Package>>;

%template(VectorChangelog) std::vector<libdnf5::rpm::Changelog>;

%rename(next) libdnf5::rpm::PackageSetIterator::operator++();
%rename(value) libdnf5::rpm::PackageSetIterator::operator*();
%include "libdnf5/rpm/package_set_iterator.hpp"
%include "libdnf5/rpm/package_set.hpp"

%ignore libdnf5::rpm::PackageQuery::PackageQuery(PackageQuery && src);
%include "libdnf5/rpm/package_query.hpp"

add_iterator(PackageSet)
add_iterator(ReldepList)

%feature("director") TransactionCallbacks;
%include "libdnf5/rpm/transaction_callbacks.hpp"
wrap_unique_ptr(TransactionCallbacksUniquePtr, libdnf5::rpm::TransactionCallbacks);

%ignore KeyImportError;
%ignore SignatureCheckError;
%include "libdnf5/rpm/rpm_signature.hpp"

%template(VectorKeyInfo) std::vector<libdnf5::rpm::KeyInfo>;
