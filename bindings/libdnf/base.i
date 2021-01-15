#if defined(SWIGPYTHON)
%module(package="libdnf") base
#elif defined(SWIGPERL)
%module "libdnf::base"
#elif defined(SWIGRUBY)
%module "libdnf/base"
#endif

%include <exception.i>
%include <std_common.i>

%import "common.i"
%import "conf.i"
%import "logger.i"
%import "rpm.i"
%import "transaction.i"

%{
    #include "libdnf/base/base.hpp"
    #include "libdnf/base/goal.hpp"
%}

#define CV __perl_CV
%include "libdnf/base/base.hpp"
%include "libdnf/base/goal.hpp"
