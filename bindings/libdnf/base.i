#if defined(SWIGPYTHON)
%module(package="libdnf") base
#elif defined(SWIGPERL)
%module "libdnf::base"
#elif defined(SWIGRUBY)
%module "libdnf/base"
#endif

%include <exception.i>
%include <std_common.i>

%include <shared.i>

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
%}

#define CV __perl_CV

%template(BaseWeakPtr) libdnf::WeakPtr<libdnf::Base, false>;
%template(LogRouterWeakPtr) libdnf::WeakPtr<libdnf::LogRouter, false>;
%template(VarsWeakPtr) libdnf::WeakPtr<libdnf::Vars, false>;

%ignore libdnf::get_pool;

%include "libdnf/base/base.hpp"
%include "libdnf/base/transaction.hpp"
%include "libdnf/base/transaction_package.hpp"
%include "libdnf/base/goal.hpp"
