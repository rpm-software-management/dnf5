%module(package="libdnf5_cli") progressbar


%include <exception.i>
%include <stdint.i>
%include <std_string.i>


%{
    // make SWIG wrap following headers
    #include "libdnf5-cli/progressbar/progress_bar.hpp"
    #include "libdnf5-cli/progressbar/download_progress_bar.hpp"
    #include "libdnf5-cli/progressbar/multi_progress_bar.hpp"
%}


#define CV __perl_CV

#define LIBDNF_CLI_API

%include "libdnf5-cli/progressbar/progress_bar.hpp"
%include "libdnf5-cli/progressbar/download_progress_bar.hpp"
%include "libdnf5-cli/progressbar/multi_progress_bar.hpp"
