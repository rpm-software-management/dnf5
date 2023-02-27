#if defined(SWIGPYTHON)
%module(package="libdnf5") base
#elif defined(SWIGPERL)
%module "libdnf5::base"
#elif defined(SWIGRUBY)
%module "libdnf5/base"
#endif

%include <exception.i>
%include <std_common.i>
%include <std_vector.i>

%include <shared.i>

%import "advisory.i"
%import "common.i"
%import "comps.i"
%import "conf.i"
%import "plugin.i"
%import "logger.i"
%import "repo.i"
%import "rpm.i"
%import "transaction.i"

%{
    #include "libdnf/logger/memory_buffer_logger.hpp"
    #include "libdnf/base/base.hpp"
    #include "libdnf/base/transaction.hpp"
    #include "libdnf/base/transaction_package.hpp"
    #include "libdnf/base/goal.hpp"
    #include "libdnf/base/goal_elements.hpp"
%}

#define CV __perl_CV

%template(BaseWeakPtr) libdnf::WeakPtr<libdnf::Base, false>;
%template(VarsWeakPtr) libdnf::WeakPtr<libdnf::Vars, false>;

%include "libdnf/base/base.hpp"
%ignore libdnf::base::TransactionError;
%include "libdnf/base/transaction.hpp"
%include "libdnf/base/transaction_package.hpp"

%template(VectorBaseTransactionPackage) std::vector<libdnf::base::TransactionPackage>;

%include "libdnf/base/goal.hpp"
%include "libdnf/base/goal_elements.hpp"
