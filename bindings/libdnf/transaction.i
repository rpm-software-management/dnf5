#if defined(SWIGPYTHON)
%module(package="libdnf") transaction
#elif defined(SWIGPERL)
%module "libdnf::transaction"
#elif defined(SWIGRUBY)
%module "libdnf/transaction"
#endif

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

%include <shared.i>

%import "common.i"

%{
    // enums
    #include "libdnf/transaction/transaction_item_action.hpp"
    #include "libdnf/transaction/transaction_item_reason.hpp"
    #include "libdnf/transaction/transaction_item_state.hpp"
    #include "libdnf/transaction/transaction_item_type.hpp"

    // transaction items
    #include "libdnf/transaction/comps_group.hpp"
    #include "libdnf/transaction/comps_environment.hpp"
    #include "libdnf/transaction/rpm_package.hpp"

    #include "libdnf/transaction/transaction_history.hpp"

    // transaction
    #include "libdnf/transaction/transaction.hpp"
%}

#define CV __perl_CV

// enums
%include "libdnf/transaction/transaction_item_action.hpp"
%include "libdnf/transaction/transaction_item_reason.hpp"
%include "libdnf/transaction/transaction_item_state.hpp"
%include "libdnf/transaction/transaction_item_type.hpp"

// transaction items
%include "libdnf/transaction/comps_group.hpp"
%include "libdnf/transaction/comps_environment.hpp"
%include "libdnf/transaction/rpm_package.hpp"

%include "libdnf/transaction/transaction_history.hpp"
%template(TransactionHistoryWeakPtr) libdnf::WeakPtr<libdnf::transaction::TransactionHistory, false>;

// transaction
%include "libdnf/transaction/transaction.hpp"
