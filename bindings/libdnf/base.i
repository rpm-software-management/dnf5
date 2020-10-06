%module base


%include <exception.i>
%include <std_common.i>

#if defined(SWIGPYTHON)
%import(module="libdnf.common") "common.i"
%import(module="libdnf.conf") "conf.i"
%import(module="libdnf.logger") "logger.i"
%import(module="libdnf.rpm") "rpm.i"
%import(module="libdnf.transaction") "transaction.i"
#elif defined(SWIGRUBY)
%import(module="libdnf/common") "common.i"
%import(module="libdnf/conf") "conf.i"
%import(module="libdnf/logger") "logger.i"
%import(module="libdnf/rpm") "rpm.i"
%import(module="libdnf/transaction") "transaction.i"
#elif defined(SWIGPERL)
%import(module="libdnf::common") "common.i"
%import(module="libdnf::conf") "conf.i"
%import(module="libdnf::logger") "logger.i"
%import(module="libdnf::rpm") "rpm.i"
%import(module="libdnf::transaction") "transaction.i"
#endif

%{
    #include "libdnf/base/base.hpp"
    #include "libdnf/base/goal.hpp"
%}

#define CV __perl_CV
%include "libdnf/base/base.hpp"
%include "libdnf/base/goal.hpp"
