#if defined(SWIGPYTHON)
%module(package="libdnf5", directors="1") logger
#elif defined(SWIGPERL)
%module(directors="1") "libdnf5::logger"
#elif defined(SWIGRUBY)
%module(directors="1") "libdnf5::logger"
#endif

%include <exception.i>
%include <std_string.i>

%include <shared.i>

%import "common.i"

%exception {
    try {
        $action
    } catch (const libdnf5::UserAssertionError & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const libdnf5::Error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const std::runtime_error & e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

typedef int64_t time_t;
typedef int32_t pid_t;

%{
    #include "libdnf5/common/weak_ptr.hpp"
    #include "libdnf5/logger/log_router.hpp"
    #include "libdnf5/logger/global_logger.hpp"
    #include "libdnf5/logger/memory_buffer_logger.hpp"
    #include "libdnf5/logger/factory.hpp"
%}

#define CV __perl_CV

wrap_unique_ptr(LoggerUniquePtr, libdnf5::Logger);
wrap_unique_ptr(MemoryBufferLoggerUniquePtr, libdnf5::MemoryBufferLogger);

%template(LogRouterWeakPtr) libdnf5::WeakPtr<libdnf5::LogRouter, false>;

%feature("director") Logger;
%include "libdnf5/logger/logger.hpp"

%extend libdnf5::Logger {
    inline void critical(const std::string & msg) { self->critical(msg); }
    inline void error(const std::string & msg) { self->error(msg); }
    inline void warning(const std::string & msg) { self->warning(msg); }
    inline void notice(const std::string & msg) { self->notice(msg); }
    inline void info(const std::string & msg) { self->info(msg); }
    inline void debug(const std::string & msg) { self->debug(msg); }
    inline void trace(const std::string & msg) { self->trace(msg); }
    inline void log(Level level, const std::string & msg) { self->log(level, msg); }
}

%include "libdnf5/logger/log_router.hpp"
%include "libdnf5/logger/global_logger.hpp"
%include "libdnf5/logger/memory_buffer_logger.hpp"
%include "libdnf5/logger/factory.hpp"
