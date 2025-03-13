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
    } catch (const std::exception &) {
        libdnf_exception_wrap_current()
        SWIG_fail;
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

%inline %{
    /// Fake function to force import of SWIG type "common.ExceptionWrap".
    libdnf5::common::ExceptionWrap _libdnf_transaction_dummy() { return libdnf5::common::ExceptionWrap(); }
%}


// enums
%ignore libdnf5::transaction::InvalidTransactionItemAction::InvalidTransactionItemAction;
%include "libdnf5/transaction/transaction_item_action.hpp"
%ignore libdnf5::transaction::InvalidTransactionItemReason::InvalidTransactionItemReason;
%include "libdnf5/transaction/transaction_item_reason.hpp"
%ignore libdnf5::transaction::InvalidTransactionItemState::InvalidTransactionItemState;
%include "libdnf5/transaction/transaction_item_state.hpp"
%ignore libdnf5::transaction::InvalidTransactionItemType::InvalidTransactionItemType;
%include "libdnf5/transaction/transaction_item_type.hpp"

// transaction items
%include "libdnf5/transaction/transaction_item.hpp"
%include "libdnf5/transaction/comps_group.hpp"
%include "libdnf5/transaction/comps_environment.hpp"
%include "libdnf5/transaction/rpm_package.hpp"

%include "libdnf5/transaction/transaction_history.hpp"
%template(TransactionHistoryWeakPtr) libdnf5::WeakPtr<libdnf5::transaction::TransactionHistory, false>;

// transaction
%ignore libdnf5::transaction::InvalidTransactionState::InvalidTransactionState;
%include "libdnf5/transaction/transaction.hpp"

%template(VectorTransaction) std::vector<libdnf5::transaction::Transaction>;
%template(VectorTransactionPackage) std::vector<libdnf5::transaction::Package>;

%exception;
