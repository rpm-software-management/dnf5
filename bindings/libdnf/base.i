%module base


%include <exception.i>

#if defined(SWIGPYTHON)
%import(module="libdnf.common") "common.i"
%import(module="libdnf.conf") "conf.i"
%import(module="libdnf.logger") "logger.i"
%import(module="libdnf.rpm") "rpm.i"
#elif defined(SWIGRUBY)
%import(module="libdnf/common") "common.i"
%import(module="libdnf/conf") "conf.i"
%import(module="libdnf/logger") "logger.i"
%import(module="libdnf/rpm") "rpm.i"
#elif defined(SWIGPERL)
%import(module="libdnf::common") "common.i"
%import(module="libdnf::conf") "conf.i"
%import(module="libdnf::logger") "logger.i"
%import(module="libdnf::rpm") "rpm.i"
#endif

%{
    #include "libdnf/base/base.hpp"
%}

#define CV __perl_CV
%include "libdnf/base/base.hpp"
