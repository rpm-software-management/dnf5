%module base


%include <exception.i>
%include <std_string.i>


%{
    // make SWIG wrap following headers
    #include "libdnf/base/base.hpp"
    using namespace libdnf;
%}

#define CV __perl_CV
%include "libdnf/base/base.hpp"
