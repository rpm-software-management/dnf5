#if defined(SWIGPYTHON)
%module(package="libdnf5") transaction
#elif defined(SWIGPERL)
%module "libdnf5::transaction"
#elif defined(SWIGRUBY)
%module "libdnf5::transaction"
#endif

%include <std_string.i>
%include <std_vector.i>

%include "shared.i"

%import "common.i"
%import "exception.i"

%{
    #include "bindings/libdnf5/exception.hpp"

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

// Deletes any previously defined general purpose exception handler
%exception;

// Set default exception handler
%catches(libdnf5::UserAssertionError, std::runtime_error, std::out_of_range);

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

// Deletes any previously defined catches
%catches();
