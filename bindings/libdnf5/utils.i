#if defined(SWIGPYTHON)
%module(package="libdnf5") utils
#elif defined(SWIGPERL)
%module "libdnf5::utils"
#elif defined(SWIGRUBY)
%module "libdnf5::utils"
#endif

%include <std_string.i>

%include "shared.i"

%import "exception.i"

%{
    #include "bindings/libdnf5/exception.hpp"

    #include "libdnf5/utils/locale.hpp"
    #include "libdnf5/utils/patterns.hpp"
    #include "libdnf5/utils/locker.hpp"
%}

#define CV __perl_CV

// Deletes any previously defined exception handlers
%exception;
%catches();

// Contains only `noexcept` functions
%include "libdnf5/utils/patterns.hpp"

// Set default exception handler
%catches(libdnf5::UserAssertionError, std::runtime_error, std::out_of_range);

%include "libdnf5/utils/locale.hpp"

// Deletes any previously defined catches
%catches();

%include "libdnf5/utils/locker.hpp"
