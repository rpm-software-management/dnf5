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
    #include "libdnf/logger/memory_buffer_logger.hpp"
    #include "libdnf/base/base.hpp"
    #include "libdnf/base/solver_problems.hpp"
    #include "libdnf/base/log_event.hpp"
    #include "libdnf/base/transaction.hpp"
    #include "libdnf/base/transaction_package.hpp"
    #include "libdnf/base/goal.hpp"
    #include "libdnf/base/goal_elements.hpp"
%}

#define CV __perl_CV

%template(BaseWeakPtr) libdnf::WeakPtr<libdnf::Base, false>;
%template(VarsWeakPtr) libdnf::WeakPtr<libdnf::Vars, false>;

%include "libdnf/base/base.hpp"

%include "libdnf/base/solver_problems.hpp"
%include "libdnf/base/log_event.hpp"

%ignore libdnf::base::TransactionError;
%include "libdnf/base/transaction.hpp"

%template(VectorLogEvent) std::vector<libdnf::base::LogEvent>;

%include "libdnf/base/transaction_package.hpp"

%template(VectorBaseTransactionPackage) std::vector<libdnf::base::TransactionPackage>;

%include "libdnf/base/goal.hpp"
%include "libdnf/base/goal_elements.hpp"
