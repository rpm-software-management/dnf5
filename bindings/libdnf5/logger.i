#if defined(SWIGPYTHON)
%module(package="libdnf5", directors="1") logger
#elif defined(SWIGPERL)
%module(directors="1") "libdnf5::logger"
#elif defined(SWIGRUBY)
%module(directors="1") "libdnf5/logger"
#endif

%include <exception.i>
%include <std_string.i>

%include <shared.i>

%import "common.i"

typedef int64_t time_t;
typedef int32_t pid_t;

%{
    #include "libdnf/common/weak_ptr.hpp"
    #include "libdnf/logger/log_router.hpp"
    #include "libdnf/logger/memory_buffer_logger.hpp"
%}

#define CV __perl_CV

wrap_unique_ptr(LoggerUniquePtr, libdnf::Logger);
wrap_unique_ptr(MemoryBufferLoggerUniquePtr, libdnf::MemoryBufferLogger);

%template(LogRouterWeakPtr) libdnf::WeakPtr<libdnf::LogRouter, false>;

%feature("director") Logger;
%include "libdnf/logger/logger.hpp"

%extend libdnf::Logger {
    inline void critical(const std::string & msg) { self->critical(msg); }
    inline void error(const std::string & msg) { self->error(msg); }
    inline void warning(const std::string & msg) { self->warning(msg); }
    inline void notice(const std::string & msg) { self->notice(msg); }
    inline void info(const std::string & msg) { self->info(msg); }
    inline void debug(const std::string & msg) { self->debug(msg); }
    inline void trace(const std::string & msg) { self->trace(msg); }
    inline void log(Level level, const std::string & msg) { self->log(level, msg); }
}

%include "libdnf/logger/log_router.hpp"
%include "libdnf/logger/memory_buffer_logger.hpp"
