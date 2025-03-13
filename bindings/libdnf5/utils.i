#if defined(SWIGPYTHON)
%module(package="libdnf5") utils
#elif defined(SWIGPERL)
%module "libdnf5::utils"
#elif defined(SWIGRUBY)
%module "libdnf5::utils"
#endif

%include <std_string.i>

%include <shared.i>

%import "common.i"

%exception {
    try {
        $action
    } catch (const std::exception &) {
        libdnf_exception_wrap_current()
        SWIG_fail;
    }
}

%{
    #include "libdnf5/utils/locale.hpp"
    #include "libdnf5/utils/patterns.hpp"
%}

#define CV __perl_CV

%inline %{
    /// Fake function to force import of SWIG type "common.ExceptionWrap".
    libdnf5::common::ExceptionWrap _libdnf_utils_dummy() { return libdnf5::common::ExceptionWrap(); }
%}


%include "libdnf5/utils/locale.hpp"
%include "libdnf5/utils/patterns.hpp"

%exception;
