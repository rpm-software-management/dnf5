#if defined(SWIGPYTHON)
%module(package="libdnf5") transaction
#elif defined(SWIGPERL)
%module "libdnf5::transaction"
#elif defined(SWIGRUBY)
%module "libdnf5::transaction"
#endif

%include <exception.i>
%include <std_string.i>
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
    // enums
    #include "libdnf5/transaction/transaction_item_action.hpp"
    #include "libdnf5/transaction/transaction_item_reason.hpp"
    #include "libdnf5/transaction/transaction_item_state.hpp"

    // transaction items
    #include "libdnf5/transaction/transaction_item.hpp"
    #include "libdnf5/transaction/comps_group.hpp"
    #include "libdnf5/transaction/comps_environment.hpp"
    #include "libdnf5/transaction/rpm_package.hpp"

    #include "libdnf5/transaction/transaction_history.hpp"

    // transaction
    #include "libdnf5/transaction/transaction.hpp"
%}

#define CV __perl_CV

// enums
%include "libdnf5/transaction/transaction_item_action.hpp"
%include "libdnf5/transaction/transaction_item_reason.hpp"
%include "libdnf5/transaction/transaction_item_state.hpp"

// transaction items
%include "libdnf5/transaction/transaction_item.hpp"
%include "libdnf5/transaction/comps_group.hpp"
%include "libdnf5/transaction/comps_environment.hpp"
%include "libdnf5/transaction/rpm_package.hpp"

%include "libdnf5/transaction/transaction_history.hpp"
%template(TransactionHistoryWeakPtr) libdnf5::WeakPtr<libdnf5::transaction::TransactionHistory, false>;

// transaction
%include "libdnf5/transaction/transaction.hpp"

%template(VectorTransaction) std::vector<libdnf5::transaction::Transaction>;
%template(VectorTransactionPackage) std::vector<libdnf5::transaction::Package>;
