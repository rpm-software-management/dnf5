#if defined(SWIGPYTHON)
%module(package="libdnf5") transaction
#elif defined(SWIGPERL)
%module "libdnf5::transaction"
#elif defined(SWIGRUBY)
%module "libdnf5/transaction"
#endif

%include <exception.i>
%include <std_string.i>
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

%{
    // enums
    #include "libdnf/transaction/transaction_item_action.hpp"
    #include "libdnf/transaction/transaction_item_reason.hpp"
    #include "libdnf/transaction/transaction_item_state.hpp"

    // transaction items
    #include "libdnf/transaction/transaction_item.hpp"
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

// transaction items
%include "libdnf/transaction/transaction_item.hpp"
%include "libdnf/transaction/comps_group.hpp"
%include "libdnf/transaction/comps_environment.hpp"
%include "libdnf/transaction/rpm_package.hpp"

%include "libdnf/transaction/transaction_history.hpp"
%template(TransactionHistoryWeakPtr) libdnf::WeakPtr<libdnf::transaction::TransactionHistory, false>;

// transaction
%include "libdnf/transaction/transaction.hpp"

%template(VectorTransaction) std::vector<libdnf::transaction::Transaction>;
%template(VectorTransactionPackage) std::vector<libdnf::transaction::Package>;
