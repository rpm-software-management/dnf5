%module transaction

%include <exception.i>
%include <std_string.i>

#if defined(SWIGPYTHON)
%import(module="libdnf.common") "common.i"
#elif defined(SWIGRUBY)
%import(module="libdnf/common") "common.i"
#elif defined(SWIGPERL)
%include "std_vector.i"
%import(module="libdnf::common") "common.i"
#endif

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

    // sack and query
    #include "libdnf/transaction/query.hpp"
    #include "libdnf/transaction/sack.hpp"

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

// sack and query
%include "libdnf/transaction/query.hpp"
%include "libdnf/transaction/sack.hpp"

// transaction
%include "libdnf/transaction/transaction.hpp"
