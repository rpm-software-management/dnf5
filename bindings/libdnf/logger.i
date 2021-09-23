#if defined(SWIGPYTHON)
%module(package="libdnf", directors="1") logger
#elif defined(SWIGPERL)
%module(directors="1") "libdnf::logger"
#elif defined(SWIGRUBY)
%module(directors="1") "libdnf/logger"
#endif

%include <exception.i>
%include <std_string.i>

%include <shared.i>

%import "common.i"

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
