#if defined(SWIGPYTHON)
%module(package="libdnf5") rpm
#elif defined(SWIGPERL)
%module "libdnf5::rpm"
#elif defined(SWIGRUBY)
%module "libdnf5::rpm"
#endif

#if defined(SWIGRUBY)
// Mixin modules for Ruby. This has to be declared before inclusion of the
// related header file.
%mixin libdnf5::rpm::PackageSet "Enumerable";
%mixin libdnf5::rpm::ReldepList "Enumerable";
#endif

%include <std_string.i>
%include <std_vector.i>

%include "shared.i"

%import "common.i"
%import "conf.i"
%import "exception.i"
%import "repo.i"
%import "transaction.i"

%{
    #include "bindings/libdnf5/exception.hpp"

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

// Deletes any previously defined general purpose exception handler
%exception;

// Set default exception handler
%catches(libdnf5::UserAssertionError, std::runtime_error, std::out_of_range);

%include "libdnf5/rpm/arch.hpp"

%include "libdnf5/rpm/checksum.hpp"

%ignore NevraIncorrectInputError;
%include "libdnf5/rpm/nevra.hpp"

%template(VectorNevra) std::vector<libdnf5::rpm::Nevra>;
%template(VectorNevraForm) std::vector<libdnf5::rpm::Nevra::Form>;
%template(PairBoolNevra) std::pair<bool, libdnf5::rpm::Nevra>;
%template(cmp_nevra) libdnf5::rpm::cmp_nevra<libdnf5::rpm::Nevra>;

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

add_ruby_each(libdnf5::rpm::PackageSet)
// Reldep needs special treatment so that the add_ruby_each can use it.
#if defined(SWIGRUBY)
fix_swigtype_trait(libdnf5::rpm::Reldep)
#endif
add_ruby_each(libdnf5::rpm::ReldepList)

%feature("director") TransactionCallbacks;
%include "libdnf5/rpm/transaction_callbacks.hpp"
wrap_unique_ptr(TransactionCallbacksUniquePtr, libdnf5::rpm::TransactionCallbacks);

%ignore KeyImportError;
%ignore SignatureCheckError;
%include "libdnf5/rpm/rpm_signature.hpp"

%template(VectorKeyInfo) std::vector<libdnf5::rpm::KeyInfo>;

// Add attributes for getters/setters in Python.
// See 'common.i' for more info.
#if defined(SWIGPYTHON)
%pythoncode %{
common.create_attributes_from_getters_and_setters(Changelog)
%}
#endif

// Deletes any previously defined catches
%catches();
