Project layout
==============

* Directories: lower_case, avoid separators if possible
* Files: snake_case.{cpp,hpp,py,...}
* Project layout::

    <top directory>
    +-- bindings                # everything related to SWIG bindings goes here
    |   +-- <language>          # go, perl5, python3, ruby
    |   |   +-- libdnf          # language specific bindings for libdnf
    |   |   +-- libdnf_cli      # language specific bindings for libdnf-cli
    |   +-- libdnf              # SWIG *.i files for libdnf
    |   +-- libdnf_cli          # SWIG *.i files for libdnf-cli
    +-- doc                     # documentation
    +-- include                 # public headers
    |   +-- libdnf              # libdnf public C++ headers
    |   +-- libdnf-cli          # libdnf-cli public C++ headers
    +-- libdnf                  # libdnf code and private headers
    +-- libdnf-cli              # libdnf-cli code and private headers
    +-- libdnf-plugins          # libdnf C/C++ plugins
    +-- dnfdaemon-client        # command line client for dnfdaemon-server
    +-- dnfdaemon-server        # DBus package manager service
    +-- microdnf                # microdnf command line package manager
    +-- test                    # tests; similar layout to the bindings
