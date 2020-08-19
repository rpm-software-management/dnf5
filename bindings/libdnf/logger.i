%module(directors="1") logger

%include <exception.i>
%include <std_string.i>

#if defined(SWIGPYTHON)
%import(module="libdnf.common") "common.i"
#elif defined(SWIGRUBY)
%import(module="libdnf/common") "common.i"
#elif defined(SWIGPERL)
%import(module="libdnf::common") "common.i"
#endif

typedef int64_t time_t;
typedef int32_t pid_t;

%{
    #include "libdnf/logger/log_router.hpp"
    #include "libdnf/logger/memory_buffer_logger.hpp"
%}

#define CV __perl_CV

wrap_unique_ptr(LoggerUniquePtr, libdnf::Logger);
wrap_unique_ptr(MemoryBufferLoggerUniquePtr, libdnf::MemoryBufferLogger);

%feature("director") Logger;
%include "libdnf/logger/logger.hpp"
%include "libdnf/logger/log_router.hpp"
%include "libdnf/logger/memory_buffer_logger.hpp"
