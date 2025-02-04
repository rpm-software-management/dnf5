#if defined(SWIGPYTHON)
%module(package="libdnf5") utils
#elif defined(SWIGPERL)
%module "libdnf5::utils"
#elif defined(SWIGRUBY)
%module "libdnf5::utils"
#endif

%include <std_string.i>

%include <shared.i>

%{
    #include "libdnf5/utils/locale.hpp"
    #include "libdnf5/utils/patterns.hpp"
%}

#define CV __perl_CV

%include "libdnf5/utils/locale.hpp"
%include "libdnf5/utils/patterns.hpp"
