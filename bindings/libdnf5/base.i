#if defined(SWIGPYTHON)
%module(package="libdnf5") base
#elif defined(SWIGPERL)
%module "libdnf5::base"
#elif defined(SWIGRUBY)
%module "libdnf5::base"
#endif

%include <exception.i>
%include <std_common.i>
%include <std_vector.i>

%include "shared.i"


%import "advisory.i"
%import "common.i"
%import "comps.i"
%import "conf.i"
%import "exception.i"
%import "plugin.i"
%import "logger.i"
%import "repo.i"
%import "rpm.i"
%import "transaction.i"

%{
    #include "bindings/libdnf5/exception.hpp"

    #include "libdnf5/logger/memory_buffer_logger.hpp"
    #include "libdnf5/base/base.hpp"
    #include "libdnf5/base/solver_problems.hpp"
    #include "libdnf5/base/log_event.hpp"
    #include "libdnf5/base/transaction.hpp"
    #include "libdnf5/base/transaction_package.hpp"
    #include "libdnf5/base/goal.hpp"
    #include "libdnf5/base/goal_elements.hpp"
%}

// Deletes any previously defined general purpose exception handler
%exception;

// Set default exception handler
%catches(libdnf5::UserAssertionError, std::runtime_error, std::out_of_range);

#define CV __perl_CV

%template(BaseWeakPtr) libdnf5::WeakPtr<libdnf5::Base, false>;
%template(VarsWeakPtr) libdnf5::WeakPtr<libdnf5::Vars, false>;

%ignore std::vector<libdnf5::plugin::PluginInfo>::insert;
%ignore std::vector<libdnf5::plugin::PluginInfo>::pop;
%ignore std::vector<libdnf5::plugin::PluginInfo>::pop_back;
%ignore std::vector<libdnf5::plugin::PluginInfo>::push;
%ignore std::vector<libdnf5::plugin::PluginInfo>::push_back;
%ignore std::vector<libdnf5::plugin::PluginInfo>::reserve;
%ignore std::vector<libdnf5::plugin::PluginInfo>::resize;
%template(VectorPluginInfo) std::vector<libdnf5::plugin::PluginInfo>;

%include "libdnf5/base/base.hpp"

%include "libdnf5/base/solver_problems.hpp"
%include "libdnf5/base/log_event.hpp"

%ignore libdnf5::base::TransactionError;
%include "libdnf5/base/transaction.hpp"

%template(VectorLogEvent) std::vector<libdnf5::base::LogEvent>;

%include "libdnf5/base/transaction_environment.hpp"
%include "libdnf5/base/transaction_group.hpp"
%include "libdnf5/base/transaction_package.hpp"

%template(VectorBaseTransactionEnvironment) std::vector<libdnf5::base::TransactionEnvironment>;
%template(VectorBaseTransactionGroup) std::vector<libdnf5::base::TransactionGroup>;
%template(VectorBaseTransactionPackage) std::vector<libdnf5::base::TransactionPackage>;

%include "libdnf5/base/goal.hpp"
%include "libdnf5/base/goal_elements.hpp"

// Add attributes for getters/setters in Python.
// See 'common.i' for more info.
#if defined(SWIGPYTHON)
%pythoncode %{
common.create_attributes_from_getters_and_setters(ResolveSpecSettings)
common.create_attributes_from_getters_and_setters(GoalJobSettings)
%}
#endif

// Deletes any previously defined catches
%catches();
