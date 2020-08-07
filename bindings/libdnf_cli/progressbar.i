%module progressbar


%include <exception.i>
%include <stdint.i>
%include <std_string.i>


%{
    // make SWIG wrap following headers
    #include "libdnf-cli/progressbar/progress_bar.hpp"
    #include "libdnf-cli/progressbar/download_progress_bar.hpp"
    #include "libdnf-cli/progressbar/multi_progress_bar.hpp"
%}


#define CV __perl_CV


%include "libdnf-cli/progressbar/progress_bar.hpp"
%include "libdnf-cli/progressbar/download_progress_bar.hpp"
%include "libdnf-cli/progressbar/multi_progress_bar.hpp"
